#!/bin/csh
unsetenv S2_TEST_SITE
unsetenv S2_LOGS_DIR
#
setenv S2_MULTI_SITE yes
#setenv S2_TEST_SITE 22DPMCERN
#setenv S2_TEST_SITE 22STORM
#setenv S2_TEST_SITE 22CASTORCERN
#setenv S2_TEST_SITE 22CASTORDEV
#setenv S2_TEST_SITE 22CASTORRAL
#setenv S2_TEST_SITE 22DCACHEFNAL
setenv S2_TEST_SITE 22DRMLBNL
#
# Please do not change the definition of the variable below.
#
setenv S2_LOGS_DIR "./s2_logs/${S2_TEST_SITE}"
#
