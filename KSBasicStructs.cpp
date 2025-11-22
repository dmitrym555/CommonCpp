#include "KBasicStructs.h"
#include "KLBasicStructs.h"

#include "KSUtils/KSUtil.h"

#include <cmath>

#include "../../CommonCpp/KSEventLog/KSEventLog.h"



bool KLParamH::isgood() {
    bool res = Quality == KLPARAM_QUALITY_GOOD;
    return res;
}


int KSPARAMWITHDESCR::getZoneStatus() {
    if ( id.paramFields.ParamType == KLIOTYPE_BOOLEAN ) {
        return ( param.Value.Boolean )? 3:-3;
    }
    else {
        double dval = this->param.getDoubleVal();
        if ( id.paramFields.useAM ) {
            if ( dval < id.paramFields.AMI )
                return -2;
            else if ( dval > id.paramFields.AMA )
                return 2;
        }
        if ( id.paramFields.usePAM ) {
            if ( dval < id.paramFields.PMI )
                return -1;
            else if ( dval > id.paramFields.PMA )
                return 1;
        }
    }
    return 0;
}


void KSRtpParamValueValue64::setValue(double dval) {
    if ( bitflags.type == KLIOTYPE_FLOAT ) {
        Value.dval = dval;
    }
    else if ( bitflags.type == KLIOTYPE_INTEGER ) {
        Value.ival = dval;
    }
    else if ( bitflags.type == KLIOTYPE_BOOLEAN ) {
        Value.bval = dval;
    }
}

std::string KSRtpParamValueValue64::getStringVal() const {
    std::string res = "0";
    if ( bitflags.type == KLIOTYPE_FLOAT ) {
        res = ksDoubleToStr( Value.dval );
    }
    else if ( bitflags.type == KLIOTYPE_INTEGER ) {
        res = strfmt( "%d", Value.ival );
    }
    else if ( bitflags.type == KLIOTYPE_BOOLEAN ) {
        res = Value.bval? "1":"0";
    }
    else {
        res = "-1";
    }
    return res;
}

double KSRtpParamValueValue64::getDVal() const {
    double res = 0;
    if ( bitflags.type == KLIOTYPE_FLOAT ) {
        return Value.dval;
    }
    else if ( bitflags.type == KLIOTYPE_INTEGER ) {
        return Value.ival;
    }
    else if ( bitflags.type == KLIOTYPE_BOOLEAN ) {
        return Value.bval;
    }
    return res;
}

uint64_t KSRtpParamValueValue64::getIVal() const  {
    uint64_t res = 0;
    if ( bitflags.type == KLIOTYPE_FLOAT ) {
        return Value.dval;
    }
    else if ( bitflags.type == KLIOTYPE_INTEGER ) {
        return Value.ival;
    }
    else if ( bitflags.type == KLIOTYPE_BOOLEAN ) {
        return Value.bval;
    }
    return res;
}

void KSRtpParamValueValue64::setValue(const KSRtpParamValueValue32& val) {
    if ( val.bitflags.type == KLIOTYPE_FLOAT )
        Value.dval = val.Value.fval;
    else if ( val.bitflags.type == KLIOTYPE_INTEGER )
        Value.ival = val.Value.ival;
    else if ( val.bitflags.type == KLIOTYPE_BOOLEAN )
        Value.bval = val.Value.bval;
}

void KSRtpParamValueValue64::setValue(const KSRtpParamValueValue64& val) {
    Value = val.Value;
}


void KSRtpParamValueValue32::setValue(const KSRtpParamValueValue64& val) {
    if ( val.bitflags.type == KLIOTYPE_FLOAT )
        Value.fval = (float)val.Value.dval;
    else if ( val.bitflags.type == KLIOTYPE_INTEGER )
        Value.ival = (int32_t)val.Value.ival;
    else if ( val.bitflags.type == KLIOTYPE_BOOLEAN )
        Value.bval = val.Value.bval;
}

void KSPARAM::AssignTo(KLPARAM& klp) {
    klp.Quality = Quality;
    klp.Flags = Flags;
    klp.Type = Type;

    if ( Type == KLIOTYPE_FLOAT ) {
        klp.Value.Float = this->Value.Double;
    }
    else if ( Type == KLIOTYPE_BOOLEAN ) {
        klp.Value.Boolean = this->Value.Boolean;
    }
    else if ( Type == KLIOTYPE_INTEGER ) {
        klp.Value.Integer = this->Value.Integer;
    }

}

void KSPARAM::Assign(const KLPARAM& other) {
    this->Quality = other.Quality;
    this->Flags = other.Flags;
    this->Type = other.Type;

    if ( Type == KLIOTYPE_FLOAT ) {
        this->Value.Double = other.Value.Float;
    }
    if ( Type == KLIOTYPE_BOOLEAN ) {
        this->Value.Boolean = other.Value.Boolean;
    }
    if ( Type == KLIOTYPE_INTEGER ) {
        this->Value.Integer = other.Value.Integer;
    }
}


void KSPARAM::Assign(const KSPARAM& other) {

    int thisType = this->Type;
    memcpy( this, &other, sizeof( KSPARAM ) );

    if ( thisType != other.Type ) {

        if ( thisType == KLIOTYPE_INTEGER ) {
            if ( other.Type == KLIOTYPE_FLOAT ) {
                this->Value.Integer = other.Value.Double;
            }
        }
        else if ( thisType == KLIOTYPE_FLOAT ) {
            if ( other.Type == KLIOTYPE_INTEGER ) {
                this->Value.Double = other.Value.Integer;
            }
        }
    }
    this->Type = thisType;
}

std::string KSPARAM::displayString() {
    std::string res;
    if ( Quality != KLPARAM_QUALITY_GOOD ) {
        return std::format( "error: quality is bad" );
    }
    return getString();
}

std::string KSPARAM::getString() {
    std::string res = "0";
    if ( Type == KLIOTYPE_FLOAT ) {
        res = std::isnan(Value.Double)? "-1": strfmt( "%.5g", Value.Double );
    }
    else if ( Type == KLIOTYPE_INTEGER ) {
        res = strfmt( "%d", Value.Integer );
    }
    else if ( Type == KLIOTYPE_BOOLEAN ) {
        res = Value.Boolean? "1":"0";
    }
    else {
        res = "-1";
    }
    return res;
}

double KSPARAM::getDoubleVal() const {
    double res = -1;
    if ( Type == KLIOTYPE_FLOAT ) {
        res = Value.Double;
    }
    else if ( Type == KLIOTYPE_INTEGER ) {
        res = Value.Integer;
    }
    else if ( Type == KLIOTYPE_BOOLEAN ) {
        res = Value.Boolean;
    }
    return res;
}

void KSPARAM::setDoubleVal( double val ) {
    if ( Type == KLIOTYPE_FLOAT ) {
        Value.Double = val;
    }
    else if ( Type == KLIOTYPE_INTEGER ) {
        Value.Integer = val;
    }
    else if ( Type == KLIOTYPE_BOOLEAN ) {
        Value.Boolean = val;
    }
}

bool KSPARAMWithCtrlId::setAddr( const std::string& straddr ) {
    std::vector<std::string> parts = split<char>( straddr, "." );
    if ( parts.size() != 4 )
        return false;
    this->rttId = std::stoi( parts[1] );
    kspd.id.paramFields.IecID = std::stoi( parts[3] );
    return true;
}





