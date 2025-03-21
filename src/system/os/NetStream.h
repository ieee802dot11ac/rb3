#ifndef OS_NETSTREAM_H
#define OS_NETSTREAM_H
#include "utl/BinStream.h"
#include "os/NetworkSocket.h"

/**
 * @brief BinStream implementation that handles networking.
 * Useful for loading rawfiles from a network or similar.
 */
class NetStream : public BinStream {
public:
    NetStream();
    virtual ~NetStream();
    virtual void Flush() {}
    virtual int Tell() { return 0; }
    virtual EofType Eof();
    virtual bool Fail() { return mFail; }
    virtual const char *Name() const;
    virtual void ReadImpl(void *, int);
    virtual void WriteImpl(const void *, int);
    virtual void SeekImpl(int, SeekType);

    void ClientConnect(const NetAddress &);
    int ReadAsync(void *, int);

    NetworkSocket *mSocket;
    bool mFail;
    float mReadTimeoutMs;
    int mBytesRead;
    int mBytesWritten;
};

#endif
