cat > amalg.c <<EOF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
EOF

cat parg/parg.h >> amalg.c

cat worgle.h >> amalg.c

cat worgle_private.h >> amalg.c

cat parg/parg.c | sed "/^#include/d" >> amalg.c

cat worgle.c | sed "/^#include/d" >> amalg.c
