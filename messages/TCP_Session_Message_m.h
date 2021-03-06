//
// Generated file, do not edit! Created by opp_msgc 4.2 from messages/TCP_Session_Message.msg.
//

#ifndef _TCP_SESSION_MESSAGE_M_H_
#define _TCP_SESSION_MESSAGE_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0402
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif



/**
 * Class generated from <tt>messages/TCP_Session_Message.msg</tt> by opp_msgc.
 * <pre>
 * packet TCP_Session_Message
 * {
 *     uint8_t canEnd;
 *     uint8_t pading[79];
 * }
 * </pre>
 */
class TCP_Session_Message : public ::cPacket
{
  protected:
    uint8_t canEnd_var;
    uint8_t pading_var[79];

  private:
    void copy(const TCP_Session_Message& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const TCP_Session_Message&);

  public:
    TCP_Session_Message(const char *name=NULL, int kind=0);
    TCP_Session_Message(const TCP_Session_Message& other);
    virtual ~TCP_Session_Message();
    TCP_Session_Message& operator=(const TCP_Session_Message& other);
    virtual TCP_Session_Message *dup() const {return new TCP_Session_Message(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual uint8_t getCanEnd() const;
    virtual void setCanEnd(uint8_t canEnd);
    virtual unsigned int getPadingArraySize() const;
    virtual uint8_t getPading(unsigned int k) const;
    virtual void setPading(unsigned int k, uint8_t pading);
};

inline void doPacking(cCommBuffer *b, TCP_Session_Message& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, TCP_Session_Message& obj) {obj.parsimUnpack(b);}


#endif // _TCP_SESSION_MESSAGE_M_H_
