#ifndef ACTIONS_H
#define ACTIONS_H

#include "../pool/pool.h"
#include "../utils/args_parser.h"
#include "../utils/common.h"

void block_action(Pool *pool, const Args *args);
void init_actions(const Args *args);

#endif // ACTIONS_H
