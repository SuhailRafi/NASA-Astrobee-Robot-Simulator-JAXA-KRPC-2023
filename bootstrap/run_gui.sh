#!/bin/sh

#####################
# Bootstrap scripts #
#####################

# update env files
bash /tmp/bootstrap/copy_objects.sh

# run
/astrobee_init.sh /bin/bash