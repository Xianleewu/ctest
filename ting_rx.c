/*-----------------------------------------------------
TODOs and BUGs
1. Ting-01M always response as busy after Widora rebooting.
   ----Solved! set serial port as RAW MODE.
2. ttyS1: 1 input overrun(s)!!  --solved.
3. 'AT+ACK=1\r\n' return with 'ERR:CMD' --ting firmware
4. Rules for ting LoRa string.
5. parse LoRa string,get data and commands, execut commands then. 


--------------------------------------------------------*/
#include   <string.h>
#include   "msg_common.h"
#include   "ting_uart.h"
#include   "ting.h"

//=================== MAIN FUNCTIONS ================
int main(int argc, char **argv)
{
  int nb,nread,nwrite;
  char tmp;

  char *pbuff;
  char *pstrTingLoraItems[MAX_TING_LORA_ITEM]; //point array to received Ting LORA itmes 
  char  STR_CFG[]="AT+CFG=434000000,10,6,7,1,1,0,0,0,0,3000,132,4\r\n";
  char *uart_dev ="/dev/ttyS1";
  //--- for IPC message------
  int msg_id=-1;
  int msg_key=MSG_KEY_OLED_TEST;

  //---- create message queue ------
  if((msg_id=createMsgQue(msg_key))<0)
  {
        printf("create message queue fails!\n");
        exit(-1);
  }

  //----- init buff and arrays ------
  //  memset(g_strUserRxBuff,'\0',sizeof(g_strUserRxBuff));
   ClearUserRxBuff(); // clear g_strUserRxBuff
   memset(pstrTingLoraItems,0,sizeof(pstrTingLoraItems));

  openUART(uart_dev); // open uart and set it.
  resetTing(g_fd, STR_CFG,0x5555, 0x6666); // reset ting with spicific parameters

  while(1)
  {
        //---- to confirm that Ting is active 
	if(checkTingActive() != 0)
	{
		 printf("checkTingActive() fails! reset Ting ...\n");
		 resetTing(g_fd, STR_CFG,0x5555, 0x6666); // reset ting with spicific parameters
	}
	//---- set RX and get LORA message
	printf("start recvTingLoRa()...\n");
	recvTingLoRa();

	//--------parse recieved data -----
	printf("start sepWordsInTingLoraStr()...\n");
	sepWordsInTingLoraStr(g_strUserRxBuff,pstrTingLoraItems);//separate key words and get total length.
	printf("start parseTingLoraWordsArray()...\n");
	parseTingLoraWordsArray(pstrTingLoraItems);//parse key words as of commands and data

	//----get RSSI
	printf("start sendTingCMD(AT+RSSI?)...\n");
	sendTingCMD("AT+RSSI?\r\n",g_ndelay);

	//---- summary ---
        printf("-----< g_intRcvCount=%d, g_intErrCount=%d  g_intMissCount=%d g_intEscapeReadLoopCount=%d >-----\n" \
	,g_intRcvCount,g_intErrCount,g_intMissCount,g_intEscapeReadLoopCount);

	//----- send data to IPC Message Queue--------
	printf("start sendMsgQue()...\n");
        if(sendMsgQue(msg_id,MSG_TYPE_TING,g_strAtBuff)!=0)
		printf("Send message queue failed!\n");

   }
    //delete IPC message queue
    //close(g_fd);
    //exit(0);
}
