/*
 *  Copyright 2001 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 */
/*
 *  ======== hello.cmd ========
 *
 */


MEMORY 
{
   ISRAM       : origin = 0x0,         len = 0x1000000
}

SECTIONS
{
        .vectors > ISRAM
        .text    > ISRAM

        .bss     > ISRAM
        .cinit   > ISRAM
        .const   > ISRAM
        .far     > ISRAM
        .stack   > ISRAM
        .cio     > ISRAM
        .sysmem  > ISRAM
}
