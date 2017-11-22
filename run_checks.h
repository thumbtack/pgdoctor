/*
 Copyright 2014-2017 Thumbtack, Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/


#ifndef RUN_CHECKS_H_
#define RUN_CHECKS_H_


#include <stddef.h>
#include "config_parser.h"

extern int run_health_checks(config_t config, char *result, size_t size);


#endif				/* RUN_CHECKS_H_ */
