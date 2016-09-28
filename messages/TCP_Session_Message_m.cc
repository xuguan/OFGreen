//
// Generated file, do not edit! Created by opp_msgc 4.2 from messages/TCP_Session_Message.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "TCP_Session_Message_m.h"

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




Register_Class(TCP_Session_Message);

TCP_Session_Message::TCP_Session_Message(const char *name, int kind) : cPacket(name,kind)
{
    this->canEnd_var = 0;
    for (unsigned int i=0; i<79; i++)
        this->pading_var[i] = 0;
}

TCP_Session_Message::TCP_Session_Message(const TCP_Session_Message& other) : cPacket(other)
{
    copy(other);
}

TCP_Session_Message::~TCP_Session_Message()
{
}

TCP_Session_Message& TCP_Session_Message::operator=(const TCP_Session_Message& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void TCP_Session_Message::copy(const TCP_Session_Message& other)
{
    this->canEnd_var = other.canEnd_var;
    for (unsigned int i=0; i<79; i++)
        this->pading_var[i] = other.pading_var[i];
}

void TCP_Session_Message::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->canEnd_var);
    doPacking(b,this->pading_var,79);
}

void TCP_Session_Message::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->canEnd_var);
    doUnpacking(b,this->pading_var,79);
}

uint8_t TCP_Session_Message::getCanEnd() const
{
    return canEnd_var;
}

void TCP_Session_Message::setCanEnd(uint8_t canEnd)
{
    this->canEnd_var = canEnd;
}

unsigned int TCP_Session_Message::getPadingArraySize() const
{
    return 79;
}

uint8_t TCP_Session_Message::getPading(unsigned int k) const
{
    if (k>=79) throw cRuntimeError("Array of size 79 indexed by %lu", (unsigned long)k);
    return pading_var[k];
}

void TCP_Session_Message::setPading(unsigned int k, uint8_t pading)
{
    if (k>=79) throw cRuntimeError("Array of size 79 indexed by %lu", (unsigned long)k);
    this->pading_var[k] = pading;
}

class TCP_Session_MessageDescriptor : public cClassDescriptor
{
  public:
    TCP_Session_MessageDescriptor();
    virtual ~TCP_Session_MessageDescriptor();

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

Register_ClassDescriptor(TCP_Session_MessageDescriptor);

TCP_Session_MessageDescriptor::TCP_Session_MessageDescriptor() : cClassDescriptor("TCP_Session_Message", "cPacket")
{
}

TCP_Session_MessageDescriptor::~TCP_Session_MessageDescriptor()
{
}

bool TCP_Session_MessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<TCP_Session_Message *>(obj)!=NULL;
}

const char *TCP_Session_MessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int TCP_Session_MessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int TCP_Session_MessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISARRAY | FD_ISEDITABLE,
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *TCP_Session_MessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "canEnd",
        "pading",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int TCP_Session_MessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='c' && strcmp(fieldName, "canEnd")==0) return base+0;
    if (fieldName[0]=='p' && strcmp(fieldName, "pading")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *TCP_Session_MessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "uint8_t",
        "uint8_t",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *TCP_Session_MessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int TCP_Session_MessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    TCP_Session_Message *pp = (TCP_Session_Message *)object; (void)pp;
    switch (field) {
        case 1: return 79;
        default: return 0;
    }
}

std::string TCP_Session_MessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    TCP_Session_Message *pp = (TCP_Session_Message *)object; (void)pp;
    switch (field) {
        case 0: return ulong2string(pp->getCanEnd());
        case 1: return ulong2string(pp->getPading(i));
        default: return "";
    }
}

bool TCP_Session_MessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    TCP_Session_Message *pp = (TCP_Session_Message *)object; (void)pp;
    switch (field) {
        case 0: pp->setCanEnd(string2ulong(value)); return true;
        case 1: pp->setPading(i,string2ulong(value)); return true;
        default: return false;
    }
}

const char *TCP_Session_MessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        NULL,
    };
    return (field>=0 && field<2) ? fieldStructNames[field] : NULL;
}

void *TCP_Session_MessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    TCP_Session_Message *pp = (TCP_Session_Message *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


