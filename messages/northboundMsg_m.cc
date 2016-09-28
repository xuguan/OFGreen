//
// Generated file, do not edit! Created by opp_msgc 4.2 from messages/northboundMsg.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "northboundMsg_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




Register_Class(northboundMsg);

northboundMsg::northboundMsg(const char *name, int kind) : cPacket(name,kind)
{
    this->sendBytes_var = 0;
    this->tdeadline_var = 0;
    this->tOpen_var = 0;
    this->tClose_var = 0;
    this->canSend_var = 0;
    this->canEnd_var = 0;
    this->clientModuleptr_var = 0;
    this->flowID_var = 0;
}

northboundMsg::northboundMsg(const northboundMsg& other) : cPacket(other)
{
    copy(other);
}

northboundMsg::~northboundMsg()
{
}

northboundMsg& northboundMsg::operator=(const northboundMsg& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void northboundMsg::copy(const northboundMsg& other)
{
    this->match_var = other.match_var;
    this->sendBytes_var = other.sendBytes_var;
    this->tdeadline_var = other.tdeadline_var;
    this->tOpen_var = other.tOpen_var;
    this->tClose_var = other.tClose_var;
    this->canSend_var = other.canSend_var;
    this->canEnd_var = other.canEnd_var;
    this->clientModuleptr_var = other.clientModuleptr_var;
    this->flowID_var = other.flowID_var;
}

void northboundMsg::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->match_var);
    doPacking(b,this->sendBytes_var);
    doPacking(b,this->tdeadline_var);
    doPacking(b,this->tOpen_var);
    doPacking(b,this->tClose_var);
    doPacking(b,this->canSend_var);
    doPacking(b,this->canEnd_var);
    doPacking(b,this->clientModuleptr_var);
    doPacking(b,this->flowID_var);
}

void northboundMsg::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->match_var);
    doUnpacking(b,this->sendBytes_var);
    doUnpacking(b,this->tdeadline_var);
    doUnpacking(b,this->tOpen_var);
    doUnpacking(b,this->tClose_var);
    doUnpacking(b,this->canSend_var);
    doUnpacking(b,this->canEnd_var);
    doUnpacking(b,this->clientModuleptr_var);
    doUnpacking(b,this->flowID_var);
}

oxm_basic_match& northboundMsg::getMatch()
{
    return match_var;
}

void northboundMsg::setMatch(const oxm_basic_match& match)
{
    this->match_var = match;
}

long northboundMsg::getSendBytes() const
{
    return sendBytes_var;
}

void northboundMsg::setSendBytes(long sendBytes)
{
    this->sendBytes_var = sendBytes;
}

simtime_t northboundMsg::getTdeadline() const
{
    return tdeadline_var;
}

void northboundMsg::setTdeadline(simtime_t tdeadline)
{
    this->tdeadline_var = tdeadline;
}

simtime_t northboundMsg::getTOpen() const
{
    return tOpen_var;
}

void northboundMsg::setTOpen(simtime_t tOpen)
{
    this->tOpen_var = tOpen;
}

simtime_t northboundMsg::getTClose() const
{
    return tClose_var;
}

void northboundMsg::setTClose(simtime_t tClose)
{
    this->tClose_var = tClose;
}

uint8_t northboundMsg::getCanSend() const
{
    return canSend_var;
}

void northboundMsg::setCanSend(uint8_t canSend)
{
    this->canSend_var = canSend;
}

uint8_t northboundMsg::getCanEnd() const
{
    return canEnd_var;
}

void northboundMsg::setCanEnd(uint8_t canEnd)
{
    this->canEnd_var = canEnd;
}

int northboundMsg::getClientModuleptr() const
{
    return clientModuleptr_var;
}

void northboundMsg::setClientModuleptr(int clientModuleptr)
{
    this->clientModuleptr_var = clientModuleptr;
}

int northboundMsg::getFlowID() const
{
    return flowID_var;
}

void northboundMsg::setFlowID(int flowID)
{
    this->flowID_var = flowID;
}

class northboundMsgDescriptor : public cClassDescriptor
{
  public:
    northboundMsgDescriptor();
    virtual ~northboundMsgDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(northboundMsgDescriptor);

northboundMsgDescriptor::northboundMsgDescriptor() : cClassDescriptor("northboundMsg", "cPacket")
{
}

northboundMsgDescriptor::~northboundMsgDescriptor()
{
}

bool northboundMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<northboundMsg *>(obj)!=NULL;
}

const char *northboundMsgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int northboundMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 9+basedesc->getFieldCount(object) : 9;
}

unsigned int northboundMsgDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<9) ? fieldTypeFlags[field] : 0;
}

const char *northboundMsgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "match",
        "sendBytes",
        "tdeadline",
        "tOpen",
        "tClose",
        "canSend",
        "canEnd",
        "clientModuleptr",
        "flowID",
    };
    return (field>=0 && field<9) ? fieldNames[field] : NULL;
}

int northboundMsgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='m' && strcmp(fieldName, "match")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sendBytes")==0) return base+1;
    if (fieldName[0]=='t' && strcmp(fieldName, "tdeadline")==0) return base+2;
    if (fieldName[0]=='t' && strcmp(fieldName, "tOpen")==0) return base+3;
    if (fieldName[0]=='t' && strcmp(fieldName, "tClose")==0) return base+4;
    if (fieldName[0]=='c' && strcmp(fieldName, "canSend")==0) return base+5;
    if (fieldName[0]=='c' && strcmp(fieldName, "canEnd")==0) return base+6;
    if (fieldName[0]=='c' && strcmp(fieldName, "clientModuleptr")==0) return base+7;
    if (fieldName[0]=='f' && strcmp(fieldName, "flowID")==0) return base+8;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *northboundMsgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "oxm_basic_match",
        "long",
        "simtime_t",
        "simtime_t",
        "simtime_t",
        "uint8_t",
        "uint8_t",
        "int",
        "int",
    };
    return (field>=0 && field<9) ? fieldTypeStrings[field] : NULL;
}

const char *northboundMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int northboundMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    northboundMsg *pp = (northboundMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string northboundMsgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    northboundMsg *pp = (northboundMsg *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getMatch(); return out.str();}
        case 1: return long2string(pp->getSendBytes());
        case 2: return double2string(pp->getTdeadline());
        case 3: return double2string(pp->getTOpen());
        case 4: return double2string(pp->getTClose());
        case 5: return ulong2string(pp->getCanSend());
        case 6: return ulong2string(pp->getCanEnd());
        case 7: return long2string(pp->getClientModuleptr());
        case 8: return long2string(pp->getFlowID());
        default: return "";
    }
}

bool northboundMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    northboundMsg *pp = (northboundMsg *)object; (void)pp;
    switch (field) {
        case 1: pp->setSendBytes(string2long(value)); return true;
        case 2: pp->setTdeadline(string2double(value)); return true;
        case 3: pp->setTOpen(string2double(value)); return true;
        case 4: pp->setTClose(string2double(value)); return true;
        case 5: pp->setCanSend(string2ulong(value)); return true;
        case 6: pp->setCanEnd(string2ulong(value)); return true;
        case 7: pp->setClientModuleptr(string2long(value)); return true;
        case 8: pp->setFlowID(string2long(value)); return true;
        default: return false;
    }
}

const char *northboundMsgDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        "oxm_basic_match",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };
    return (field>=0 && field<9) ? fieldStructNames[field] : NULL;
}

void *northboundMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    northboundMsg *pp = (northboundMsg *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getMatch()); break;
        default: return NULL;
    }
}


