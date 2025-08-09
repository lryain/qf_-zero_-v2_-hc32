#ifndef trans_task_H
#define trans_task_H

#include "user.h"
#include "trans_packer.h"

void esp_trans_init(void);

trans_packer_handle_t *esp_trans_get_handle(void);


#endif
