#ifndef ports_h
#define ports_h
#include "types.h"
#include "list.h"

struct MsgPort
{
  struct Node  mp_Node;
  struct Task *mp_SigTask;
  struct List  mp_MsgList;
  UINT8        mp_Flags;
  UINT8        mp_SigBit;
  UINT16       mp_Padding;
};

struct Message
{
  struct Node      mn_Node;
  struct MsgPort  *mn_ReplyPort;
  UINT32           mn_Length;
};

#define IsMsgPortEmpty(x) \
	( ((x)->mp_MsgList.lh_TailPred) == (struct Node *)(&(x)->mp_MsgList) )

#define PA_MASK     3
#define PA_UNKNOWN  3
#define PA_SIGNAL   0
#define PA_SOFTINT  1
#define PA_IGNORE   2


#endif
