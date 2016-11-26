#ifndef __UPDATE_H__
#define __UPDATE_H__

void send_first_heartbeat();
void update(unsigned long deployment_id);
unsigned long get_deployment_id();

#endif
