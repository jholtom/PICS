
#ifndef _ELYSIUM_RF_DLL_H_
#define _ELYSIUM_RF_DLL_H_

#ifdef __cplusplus
extern "C" {
#endif
  void elyRFDLLHandlePacket(void);
  void elyRFDLLHandleRxFifo(SX1212Driver * devp);
  void elyRFDLLRxInit(SX1212Driver * devp);
  void elyRFDLLTxInit(SX1278Driver * devp);
  void elyRFDLLInitiateTransmit(SX1278Driver * devp);
  void elyRFDLLHandleTxFifo(SX1278Driver * devp);
  void elyRFDLLBuildFrame(void);
#ifdef __cplusplus
}
#endif

#endif
