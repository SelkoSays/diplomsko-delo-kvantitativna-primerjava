#ifndef ACTIONS_HPP
#define ACTIONS_HPP

#include "../../c/utils/args_parser.h"
#include "../../c/utils/common.h"

#include "../pool/pool.hpp"
#include "../random/random.hpp"

namespace action {

void block_action(Pool &pool, const Args &args, Random &rng);
void init_actions(const Args &args);

} // namespace action

#endif // ACTIONS_HPP
