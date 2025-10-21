//
// C++ common units
// Author: Dmitry Melnik
//

#include "KSRTPBridge.h"


void KSRtpFromKSValue( KSRtpParamValueValue64& rtpv, const KSPARAM& ksp ) {
    if ( ksp.Type == KLIOTYPE_INTEGER ) {
        rtpv.Value.ival = ksp.Value.Integer;
    }
    else if ( ksp.Type == KLIOTYPE_FLOAT) {
        rtpv.Value.dval = ksp.Value.Double;
    }
    else if ( ksp.Type == KLIOTYPE_BOOLEAN ) {
        rtpv.bitflags.discretVal = ksp.Value.Boolean? 1:0;
        rtpv.Value.bval = ksp.Value.Boolean;
    }
}

