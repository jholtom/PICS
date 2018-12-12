
#include "channels.h"

static uint8_t PERSIST rssi;
static uint16_t PERSIST output_power;
static uint32_t PERSIST rx_frame_count;
static uint32_t PERSIST tx_frame_count;
static uint32_t PERSIST rx_pkt_count;
static uint32_t PERSIST tx_pkt_count;
static uint32_t PERSIST rf_rx_bytes_count;
static uint32_t PERSIST rf_tx_bytes_count;
static uint32_t PERSIST uart_rx_bytes_count;
static uint32_t PERSIST uart_tx_bytes_count;
static uint32_t PERSIST cmd_recieved_count;
static uint32_t PERSIST reply_sent_count;
static int8_t PERSIST mcu_temp;
static int8_t PERSIST pa_temp;
static int8_t PERSIST hs_temp;
static int32_t PERSIST last_gnd_contact;
static int32_t PERSIST last_sc_contact;

/* At one point I thought this needed to be all 1's... not sure why */
static interval_t PERSIST intervals[ChanCoreMAX + ChanNLMAX + ChanDLLMAX];

static void interval_alloc(size_t size, unsigned align) {
  static PERSIST uint8_t * storage = intervals;
  uint8_t * alloc = storage;
  if (alloc >= intervals + 
      (sizeof(interval_t) * (ChanCoreMAX + ChanNLMAX + ChanDLLMAX))) {
    return NULL;
  }
  
  storage += sizeof(interval_t);
  return alloc;
}

static PERSIST MEMORYPOOL_DECL(interval_mpool, sizeof(interval_t), interval_alloc);

static interval_t * PERSIST first_interval = NULL;
static interval_t * PERSIST current_interval = NULL;

#define LOG_CHAN_NODE(index, data) \
{\
  &logged_channels[index],\
  &data,\
  &logged_channels[index],\
  sizeof(data)-1,\
  index\
}

static channel_node_t PERSIST logged_channels[ChanCoreMax + ChanNLMax + ChanDLLMax] = {
  LOG_CHAN_NODE(0, rssi),
  LOG_CHAN_NODE(1, output_power),
  LOG_CHAN_NODE(2, rx_frame_count),
  LOG_CHAN_NODE(3, tx_frame_count),
  LOG_CHAN_NODE(4, rx_pkt_count),
  LOG_CHAN_NODE(5, tx_pkt_count),
  LOG_CHAN_NODE(6, rf_rx_bytes_count),
  LOG_CHAN_NODE(7, rf_tx_bytes_count),
  LOG_CHAN_NODE(8, uart_rx_bytes_count),
  LOG_CHAN_NODE(9, uart_tx_bytes_count),
  LOG_CHAN_NODE(10, cmd_recieved_count),
  LOG_CHAN_NODE(11, reply_sent_count),
  LOG_CHAN_NODE(12, mcu_temp),
  LOG_CHAN_NODE(13, pa_temp),
  LOG_CHAN_NODE(14, hs_temp),
  LOG_CHAN_NODE(15, last_gnd_contact),
  LOG_CHAN_NODE(16, last_sc_contact),
  NL_CHAN_INIT,
  DLL_CHAN_INIT
};
static channel_node_t PERSIST reported_channels[ChanCoreMax + ChanNLMax + ChanDLLMax] = {
  LOG_CHAN_NODE(0, rssi),
  LOG_CHAN_NODE(1, output_power),
  LOG_CHAN_NODE(2, rx_frame_count),
  LOG_CHAN_NODE(3, tx_frame_count),
  LOG_CHAN_NODE(4, rx_pkt_count),
  LOG_CHAN_NODE(5, tx_pkt_count),
  LOG_CHAN_NODE(6, rf_rx_bytes_count),
  LOG_CHAN_NODE(7, rf_tx_bytes_count),
  LOG_CHAN_NODE(8, uart_rx_bytes_count),
  LOG_CHAN_NODE(9, uart_tx_bytes_count),
  LOG_CHAN_NODE(10, cmd_recieved_count),
  LOG_CHAN_NODE(11, reply_sent_count),
  LOG_CHAN_NODE(12, mcu_temp),
  LOG_CHAN_NODE(13, pa_temp),
  LOG_CHAN_NODE(14, hs_temp),
  LOG_CHAN_NODE(15, last_gnd_contact),
  LOG_CHAN_NODE(16, last_sc_contact)
  NL_CHAN_INIT,
  DLL_CHAN_INIT
};

static PERSIST BSEMAPHORE_DECL(buffer_ready, true);

static void timer_cb(GPTDriver * gptp) {
  
  if (log_len > 0) {
    /* TODO log buffer once telem exists */
  }
  if (report_len > 0) {
    /* Post buffer */
    elyUARTPostI(report_buffer);
  }
  log_len = 0;
  report_len = 0;
  
  /* Get the next interval */
  if (current_interval->next != NULL) {
    gptChangeIntervalI(&chan_gpt, 
      current_interval->next->interval - current_interval->interval);
    current_interval = current_interval->next;
  }
  else {
    current_interval = first_interval;
    gptChangeIntervalI(&chan_gpt, current_interval->interval);
  }
  
}

static const GPTConfig PERSIST timer_cfg = {
  10,
  timer_cb
};

interval_t * get_interval(uint32_t input) {
  /* Find the existing interval or create a new interval slot */
  interval_t * curr = first_interval;
  if (curr == NULL) {
    /* Create new list */
    /* Get a new interval from the pool */
    interval_t * new = chPoolAlloc(&interval_mpool);
    chDbgAssert(new != NULL, "Too many subscriptions!");
    chDbgAssert(new->next == NULL && new->prev == NULL, 
        "Didn't zero interval before freeing");
    
    new->interval = input;
    first_interval = new;
    return new;
  }
  /* Walk the list looking for the right spot */
  while (curr->next != NULL && curr->next->interval < input) {
    curr = curr->next;
  }
  
  if (curr->next->interval == input) {
    /* Found existing interval */
    return curr->next;
  }
  
  /* Get a new interval from the pool */
  interval_t * new = chPoolAlloc(&interval_mpool);
  chDbgAssert(new != NULL, "Too many subscriptions!");
  chDbgAssert(new->next == NULL && new->prev == NULL, 
      "Didn't zero interval before freeing");
  
  new->interval = input;
  
  /* Add to list between curr and curr->next */
  new->next = curr->next;
  curr->next = new;
  
  return new;

}

uint8_t index_from_id(uint8_t chan_id) {
  osalDbgAssert(chan_id >= 0x40 && chan_id < 0x80, "invalid channel id");
  
  if (chan_id < ChanCoreMAX) { /* core channels */
    return chan_id & 0x3F;
  }
  if (chan_id < ChanNLMax) { /* network channels */
    return (chan_id - 0x60) + (ChanCoreMAX & 0x3F);
  }
  /* data link channels */
  return (chan_id - 0x70) + (ChanCoreMax & 0x3F) + (ChanNLMax - 0x60);
}

void unsubscribe_channel(uint8_t chan_id) {
  remove_node(&reported_channels[index_from_id(chan_id)]);
}

void unlog_channel(uint8_t chan_id) {
  remove_node(&logged_channels[index_from_id(chan_id)]);
}

void remove_interval(channel_node_t * node) {
  /* Walk the list, look for the node */
  interval_t * curr = first_interval;
  
  chDbgAssert(curr != NULL);
  if (curr->chan_list == node) {
    /* Remove first interval */
    first_interval = curr->next;
    chPoolFree(&interval_mpool, curr);
  }
  
  /* Find the interval to remove */
  while (curr->next != NULL && curr->next->chan_list != node) {
    curr = curr->next;
  }
  
  chDbgAssert(curr->next != NULL);
  /* Remove the following node */
  interval_t * next = curr->next;
  curr->next = curr->next->next;
  chPoolFree(&interval_mpool, curr->next);
}

void remove_node(channel_node_t * node) {
  chDbgAssert(node != NULL, "Removing NULL is not allowed");
  if (node->prev == node) {
    /* Not subscribed */
    chDbgAssert(node->next == node, "Asymmetric removal");
    return;
  }
  if (node->prev == NULL && node->next == NULL) {
    /* Last node in interval - remove the orphan */
    remove_interval(node);
  }
  
  /* Remove node from list */
  if (node->prev != NULL) {
    /* OK if next is NULL */
    node->prev->next = node->next;
  }
  if (node->next != NULL) {
    /* OK if prev is NULL */
    node->next->prev = node->prev;
  }
  
  /* Disable the node */
  node->prev = node;
  node->next = node;
}

bool node_enabled(channel_node_t * node) {
  chDbgAssert(node != NULL, "Checking if NULL enabled");
  return !(node->prev == NULL && node->next == NULL);
}

void enable_channel(uint8_t chan_id, 
                    channel_node_t * channels, 
                    interval_t * interval) {
  
  uint8_t chan_index = index_from_id(chan_id);
  /* If the channel is already subscribed, remove it from the old interval */
  if (node_enabled(&channels[chan_index])) {
    remove_node(&channels[chan_index]);
    elyErrorSignal(ErrSubOverwrite);
  }
  
  /* Add the channel to the interval list */
  channel_node_t * curr = interval->chan_list;
  
  /* Nothing in the list? Create it */
  if (NULL == curr) {
    interval->chan_list = &channels[chan_index];
  }
  else {
    /* Insert at front of list */
    chDbgAssert(NULL == curr->prev, "invalid channel list insertion");
    curr->prev = &channels[chan_index];
    channels[chan_index].next = curr;
    channels[chan_index].prev = NULL; /* TODO too paranoid? */
  }
}

void elyChanSubscribe(uint8_t * buffer, uint8_t length, uint32_t interval) {
  interval_t * i = get_interval(interval);
  
  for (int i = 0; i < length; i++) {
    enable_channel(buffer[i], reported_channels, interval);
  }
  
  if (chan_gpt.state != GPT_CONTINUOUS) {
    gptStartContinuous(&chan_gpt, interval & 0xFF);
    current_interval = interval;
  }
  else if (gptGetIntervalX(&chan_gpt) - gptGetCounterX(&chan_gpt) > interval) {
    chDbgAssert(interval < 0x0000FFFF, "This math makes no sense");
    gptChangeInterval(&chan_gpt, interval);
    current_interval = interval;
  }
}

void elyChanUnsubscribe(uint8_t * buffer, uint8_t length) {
  for (int i = 0; i < length; i++) {
    unsubscribe_channel(buffer[i]);
  }
  
}

void elyChanLog(uint8_t * buffer, uint8_t length, uint32_t interval) {
  interval_t * i = get_interval(interval);
  
  for (int i = 0; i < length; i++) {
    enable_channel(buffer[i], logged_channels, interval);
  }
  
  if (chan_gpt.state != GPT_CONTINUOUS) {
    gptStartContinuous(&chan_gpt, interval & 0xFF);
    current_interval = interval;
  }
  else if (gptGetIntervalX(&chan_gpt) - gptGetCounterX(&chan_gpt) > interval) {
    chDbgAssert(interval < 0x0000FFFF, "This math makes no sense");
    gptChangeInterval(&chan_gpt, interval);
    current_interval = interval;
  }
}

void elyChanUnlog(uint8_t * buffer, uint8_t length) {
  for (int i = 0; i < length; i++) {
    unlog_channel(buffer[i]);
  }
  
}

size_t elyChanGetValue(uint8_t * buffer, uint8_t id) {
  uint8_t chan_index = index_from_id(id);
  
  size_t n = logged_channels[chan_index].size;
  
  for (int i = 1; i < n+1; i--) {
    buffer[i] = (uint8_t *)(reported_channels[chan_index].chan_data)[n-i-1];
  }
  
  return n;
  
}

void elyChanReset() {
  interval_t * interval = first_interval;
  while (curr != NULL) {
    channel_node_t * node = interval->chan_list;
    /* Walk whole list and remove all nodes */
    while (curr != NULL) {
      if (curr.prev != NULL) {
        curr.prev.next = NULL;
      }
      curr.prev = NULL;
      curr = curr.next;
    }
    
    /* Remove from the interval list */
    interval->interval = 0;
    interval->prev = NULL;
    interval = interval->next;
    interval->prev->next = NULL;
  }
}

THD_WORKING_AREA(waChanThd, 128);
THD_FUNCTION(ChanThd, arg) {
  
  /* "On Reset" code goes here */
  /* Re-initialize timer */
  gptStart(&chan_gpt, &timer_cfg);
  
  /* If the timer was running, restart it */
  /* TODO think if there's a way to preserve the running timer */
  if (current_interval != NULL) {
    if (current_interval->next != NULL) {
      gptStartContinuous(&chan_gpt, 
        current_interval->next->interval - current_interval->interval);
      current_interval = current_interval->next;
    }
    else {
      current_interval = first_interval;
      gptStartContinuous(&chan_gpt, current_interval->interval);
    }
  }
  
  while (MSG_OK == chBSemWait(&buffer_ready)) {
    /* Loop through the channel list of the current (next) interval */
    channel_node_t * curr = current_interval->chan_list;
    uint8_t * buffer;
    uint8_t * len;
    /* TODO not currently handling addressing or headers */
    while (curr != NULL) {
      if (curr->logged) {
        /* Log channel */
        len = &log_len;
        buffer = log_buffer + log_len;
      }
      else {
        /* Report channel */
        len = &report_len;
        buffer = report_buffer + report_len;
      }
    
      switch (curr->chan_id) {
        case ChanRSSI:
        case ChanMCUTemp:
        case ChanPATemp:
        case ChanHSTemp:
          n = 1;
          break;
        case ChanOutputPower:
          n = 2;
          break;
        case ChanUplinkFrames:
        case ChanDownlinkFrames:
        case ChanUplinkPackets:
        case ChanDownlinkPackets:
        case ChanRadioRXBytes:
        case ChanRadioTXBytes:
        case ChanUARTRXBytes:
        case ChanUARTTXBytes:
        case ChanCmdReceivedCount:
        case ChanReplySentCount:
        case ChanLastGroundContact:
        case ChanLastSCContact:
          n = 4;
          break;
        default:
          chDbgAssert(false, "shouldn't happen");
      }
      
      buffer[0] = curr->chan_id;
      
      for (int i = n-1; i >= 0; i--) {
        buffer[n-i] = (uint8_t *)(curr->chan_data)[i];
      }
      
      (*len) += n;
    
    }
    
    /* Block until the ISR triggers */
    
  }
  
  /* TODO handle timer overruns more rationally */
  chDbgAssert(false, "timer overrun maybe? not sure");
  
}
