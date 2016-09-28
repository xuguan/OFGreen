//
// Generated file, do not edit! Created by opp_msgc 4.2 from messages/northboundMsg.msg.
//

#ifndef _NORTHBOUNDMSG_M_H_
#define _NORTHBOUNDMSG_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0402
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif

// cplusplus {{
#include <omnetpp.h>
    #include "openflow.h"
    #include "IPvXAddress.h"
// }}



/**
 * Class generated from <tt>messages/northboundMsg.msg</tt> by opp_msgc.
 * <pre>
 * packet northboundMsg
 * {
 * 
 * 
 * 
 * 
 *     oxm_basic_match match; 
 *     long sendBytes;
 *     simtime_t tdeadline;
 *     simtime_t tOpen;
 *     simtime_t tClose;
 *     uint8_t canSend;
 *     uint8_t canEnd;
 *     int clientModuleptr;
 *     int flowID;
 *     
 * }
 * </pre>
 */
class northboundMsg : public ::cPacket
{
  protected:
    oxm_basic_match match_var;
    long sendBytes_var;
    simtime_t tdeadline_var;
    simtime_t tOpen_var;
    simtime_t tClose_var;
    uint8_t canSend_var;
    uint8_t canEnd_var;
    int clientModuleptr_var;
    int flowID_var;

  private:
    void copy(const northboundMsg& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const northboundMsg&);

  public:
    northboundMsg(const char *name=NULL, int kind=0);
    northboundMsg(const northboundMsg& other);
    virtual ~northboundMsg();
    northboundMsg& operator=(const northboundMsg& other);
    virtual northboundMsg *dup() const {return new northboundMsg(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual oxm_basic_match& getMatch();
    virtual const oxm_basic_match& getMatch() const {return const_cast<northboundMsg*>(this)->getMatch();}
    virtual void setMatch(const oxm_basic_match& match);
    virtual long getSendBytes() const;
    virtual void setSendBytes(long sendBytes);
    virtual simtime_t getTdeadline() const;
    virtual void setTdeadline(simtime_t tdeadline);
    virtual simtime_t getTOpen() const;
    virtual void setTOpen(simtime_t tOpen);
    virtual simtime_t getTClose() const;
    virtual void setTClose(simtime_t tClose);
    virtual uint8_t getCanSend() const;
    virtual void setCanSend(uint8_t canSend);
    virtual uint8_t getCanEnd() const;
    virtual void setCanEnd(uint8_t canEnd);
    virtual int getClientModuleptr() const;
    virtual void setClientModuleptr(int clientModuleptr);
    virtual int getFlowID() const;
    virtual void setFlowID(int flowID);
};

inline void doPacking(cCommBuffer *b, northboundMsg& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, northboundMsg& obj) {obj.parsimUnpack(b);}


#endif // _NORTHBOUNDMSG_M_H_
