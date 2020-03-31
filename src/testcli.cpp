#include "cs_client.h"

int main(){
    CSClient* pclient = new CSClient();
    pclient->SetTimeout(200);
    pclient->SetAddress("127.0.0.1",6666);

    //3. push data into state machine all the time
    clock_t cur_time = clock();
    clock_t last_send_time = cur_time;
    const uint32_t SEND_INTERVAL=CLOCKS_PER_SEC/20000;
    while(true)
    {  
        cur_time = clock();
        if(cur_time - last_send_time > SEND_INTERVAL)
        {
            CSProtoPacket cspkt;
            cspkt.Head().version = 22;
            cspkt.Head().uid = 11111111111;
            std::string strbuf;
            cspkt.EncodeToString(&strbuf);
            //ping
            if (!pclient->SendPkg(strbuf.data(),strbuf.size()))
            {
                LOG_ERROR("fd: %d send fail",pclient->GetFd());
            }
            last_send_time = cur_time;
        }
    }
    return 0;
}
