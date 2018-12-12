

.state0 :
{
  . = ALIGN(2);
  *(.state)
} > FRAM

STARTOF_STATE0 = LOADADDR(.state0);

.state1 LOADADDR(.state0) + (1 << LOG2CEIL(SIZEOF(.state0))) :
{
  SHORT(0xFFFF);
} > FRAM

STARTOF_STATE1 = LOADADDR(.state1);
