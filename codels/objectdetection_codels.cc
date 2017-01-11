#include "acobjectdetection.h"

#include "objectdetection_c_types.h"


/* --- Function Stop ---------------------------------------------------- */

/** Codel codelStop of function Stop.
 *
 * Returns genom_ok.
 */
genom_event
codelStop(int32_t *stopflag, genom_context self)
{
  *stopflag = 1;
  return genom_ok;
}
