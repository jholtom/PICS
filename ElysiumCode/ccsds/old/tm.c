

void process_tf(void* frame) {
  mc = get_next_mc();
  if (mc->ocf) {
    frame->ocf = mc->ocf;
    frame->header->ocf = 1;
  }
  if (mc->fsh) {
    frame->fsh = mc->fsh;
    frame->header->flags |= SH_FLAG;
    /* something to do with sizes here */
  }
  if (mc->mcf) {
    frame->payload = mc->mcf;
  }
  else {
    vc = get_next_vc(mc);
    if (!frame->ocf) {
      frame->ocf = vc->ocf;
      frame->header->ocf = 1;
    }
    /* error reporting */
    if (!frame->fsh) {
      frame->fsh = vc->fsh;
      frame->header->flags |= SH_FLAG;
    }
    /* error reporting */
    if (vc->vcf) {
      frame->payload = vc->vcf;
    }
    else if (vc->vca_mailbox_has_stuff) { /* obvious pseudo is obvious */
      frame->payload = vc->vca_mailbox_get; /* fill frames and stuff here */
      frame->header->flags |= SYNCH_FLAG;
    }
    else if (vc->vcp_mailbox_has_stuff) {
      frame->payload = vc->vcp_mailbox_get; /* fill packets here */
    }
    else {
      /* fill stuff and/or error reporting */
    }
    vc->frame_count++;
    frame->ph->vcfc = vc->frame_count;
    frame->ph->vcid = vc->id;
  }
    
  mc->frame_count++;
  frame->ph->mcfc = mc->frame_count;
  frame->ph->mcid = mc->id;

}
