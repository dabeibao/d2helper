#pragma once

#include <stdio.h>
#include <string.h>

class FileScanner {
    static constexpr    int BufSize = 128 * 1024;
    static constexpr    int MaxLineSize = 1024;

public:
    FileScanner(const char * fileName = nullptr): mFp(nullptr), mBuf(nullptr), mCur(nullptr), mLen(0)
    {
        if (fileName != nullptr) {
            open(fileName);
        }
    }
    ~FileScanner()
    {
        if (mBuf != nullptr) {
            delete [] mBuf;
        }
        if (mFp != nullptr) {
            fclose(mFp);
        }
    }

    bool        open(const char * fileName)
    {
        close();

        initBuf();
        if (mBuf == nullptr) {
            return false;
        }
        FILE *  fp = fopen(fileName, "r");
        if (fp == nullptr) {
            return false;
        }
        mFp = fp;
        return true;
    }

    void        close()
    {
        if (isOpen()) {
            fclose(mFp);
            mFp = nullptr;
        }
    }
    bool        isOpen() { return mFp != nullptr; }
    char *      getLine()
    {
        if (!isOpen()) {
            return nullptr;
        }

        for (;;) {
            char *      s = getLineFromBuf();

            if (s != nullptr) {
                return s;
            }

            if (remaining() > MaxLineSize) {
                char *  s = mCur;
                mCur[MaxLineSize - 1] = '\0';
                mCur += MaxLineSize;
                return s;
            }

            if (feof(mFp) || ferror(mFp)) {
                return finishBuf();
            }

            int         ret = load();
            if (ret <= 0) {
                return finishBuf();
            }
        }

        return nullptr;
    }

private:
    void        initBuf()
    {
        if (mBuf == nullptr) {
            // 1 byte for terminator
            mBuf = new char[BufSize + 1];
            if (mBuf == nullptr) {
                return;
            }
        }
        mLen = 0;
        mCur = mBuf;
    }

    char *      finishBuf()
    {
        int     len = remaining();
        char *  s = mCur;

        mLen = 0;
        mCur = mBuf;
        return len > 0? s : nullptr;
    }

    char *      getLineFromBuf()
    {
        char * s = mCur;
        for (char * p = mCur; p < mBuf + mLen; p += 1) {
            if (*p == '\n' || *p == '\0') {
                mCur = p + 1;
                *p = '\0';
                return s;
            }
        }

        return nullptr;
    }

    int         load()
    {
        int     moveLen = remaining();

        if (moveLen != 0) {
            memmove((void *)mBuf, mCur, moveLen);
        }
        mLen = moveLen;
        mCur = mBuf;
        mBuf[mLen] = '\0';

        int     readSize = BufSize - mLen;
        int     ret = fread(&mBuf[mLen], 1, readSize, mFp);

        if (ret < 0) {
            return -1;
        }

        mLen += ret;
        mBuf[mLen] = '\0';
        return ret;
    }

    int         remaining()
    {
        return mLen - (mCur - mBuf);
    }


private:
    FILE *      mFp;
    char *      mBuf;
    char *      mCur;
    int         mLen;
};

class LineScan {
public:
    LineScan(char * buf): mBuf(buf)
    {
    }

    char *      getLine()
    {
        char * cur = mBuf;
        if (mBuf == nullptr) {
            return mBuf;
        }
        char * nl = strchr(mBuf, '\n');
        if (nl != nullptr) {
            *nl = '\0';
            mBuf = nl + 1;
        } else {
            mBuf = nullptr;
        }

        return cur;
    }

private:
    char *      mBuf;
};
