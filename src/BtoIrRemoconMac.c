#if defined( __APPLE__ )
/**
 *
 * BtoIrRemoconMac.c
 *
 * 2013 KLab Inc.
 *
 * ビット・トレード・ワン社製「USB 接続赤外線リモコンキット」を操作する
 * Mac OS X 用 コンソールプログラム
 *
 * 同社製品ページ:
 * http://bit-trade-one.co.jp/BTOpicture/Products/005-RS/index.html
 *
 * ビルド方法:
 * gcc -Wall -g BtoIrRemoconMac.c -framework IOKit -framework CoreFoundation -o BtoIrRemoconMac
 *  - gcc 4.2.1 build 5666 (Xcode 3.2.6) でのビルドを確認
 *
 * HID Class Device Interfaces Guide - developer.apple.com
 * http://developer.apple.com/library/mac/#documentation/DeviceDrivers/Conceptual/HID/intro/intro.html
 *
 *
 * This software is provided "as is" without any express and implied warranty
 * of any kind. The entire risk of the quality and performance of this software
 * with you, and you shall use this software your own sole judgment and
 * responsibility. KLab shall not undertake responsibility or liability for
 * any and all damages resulting from your use of this software.
 * KLab does not warrant this software to be free from bug or error in
 * programming and other defect or fit for a particular purpose, and KLab does
 * not warrant the completeness, accuracy and reliability and other warranty
 * of any kind with respect to result of your use of this software.
 * KLab shall not be obligated to support, update or upgrade this software. 
 *
 */

 /*
  使い方：
 
  0. PC に USB 接続赤外線リモコンキット（以下"USB IR REMOCON"）を接続
 
  1.リモコンデータ受信モード
  BtoIrRemoconMac を引数なしで実行するとリモコンデータ受信モードで起動する。
  手持ちの赤外線リモコンから USB IR REMOCON の赤外線 LED に向けて赤外線
  データを送るとコンソールに 14 文字のリモコンデータコードが表示される。
  例：C10220B0008030
 
  2.リモコンデータ送信モード
  BtoIrRemoconMac を上記 1. のコードを引数として起動すると USB IR REMOCON
  の赤外線 LED からそのリモコンデータが照射される。コードは複数指定可能、
  途中に p<秒数> の要領でポーズ指定を記述できる。
  例：BtoIrRemoconMac C10220B0008030 p3 C10220B0008034 p3 C10220B0008036

 */

#include <stdio.h>
#include <unistd.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <CoreFoundation/CoreFoundation.h>

#define RECEIVE_WAIT_MODE_NONE  0
#define RECEIVE_WAIT_MODE_WAIT  1

#define DEVICE_BUFSIZE       65
#define REMOCON_DATA_LENGTH   7

static int g_readBytes;

// 指定されたキーの整数プロパティを取得
int getIntProperty(IOHIDDeviceRef inIOHIDDeviceRef, CFStringRef inKey) {
    int val;
       if (inIOHIDDeviceRef) {
        CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty(inIOHIDDeviceRef, inKey);
        if (tCFTypeRef) {
            if (CFNumberGetTypeID() == CFGetTypeID(tCFTypeRef)) {
                if (!CFNumberGetValue( (CFNumberRef) tCFTypeRef, kCFNumberSInt32Type, &val)) {
                    val = -1;
                }
            }
        }
    }
    return val;
}

// レポートのコールバック関数
static void reportCallback(void *inContext, IOReturn inResult, void *inSender,
                           IOHIDReportType inType, uint32_t inReportID,
                           uint8_t *inReport, CFIndex InReportLength)
{
    g_readBytes = InReportLength;
}

// デバイスからの読み込み
int ReadFromeDevice(IOHIDDeviceRef dev, unsigned char *buf, size_t bufsize, CFTimeInterval timeoutSecs)
{
    IOHIDDeviceRegisterInputReportCallback(dev,
                                       &buf[1],
                                       bufsize-1,
                                       reportCallback,
                                       NULL);
    g_readBytes = -1;
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, timeoutSecs, false);
    //printf("ReadFromeDevice: len=%d, 0=%X 1=%X 2=%X\n", g_readBytes, buf[0], buf[1], buf[2]);
    return g_readBytes;
}

// デバイスへの書き込み
IOReturn WriteToDevice(IOHIDDeviceRef dev, unsigned char *data, size_t len)
{
    IOReturn ret = IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, data[0], data+1, len-1);
    if (ret != kIOReturnSuccess) {
        //printf("WriteToDevice: ret=0x%08X\n", ret);
    }
    return ret;
}

// リモコンデータ送信モード
int Transfer(IOHIDDeviceRef refDevice, int ac, char *av)
{
    int i;
    unsigned char buf[DEVICE_BUFSIZE];
    
    memset(buf, 0xFF, sizeof(buf));
    buf[0] = 0;
    
    // パラメータをコピー
    for (i = 0; i < ac; i++) {
    	buf[i+1] = *av;
    	++av;
    }
    if (WriteToDevice(refDevice, buf, DEVICE_BUFSIZE) != kIOReturnSuccess) {
          fprintf(stderr, "WriteToDevice: err\n");
    }
    return 1;
}

// リモコンデータ受信モード
int Display(IOHIDDeviceRef refDevice)
{
    int i, sts = -1;
    unsigned char buf[DEVICE_BUFSIZE];
    
    // デバイスの送信バッファをクリア
    memset(buf, 0xFF, sizeof(buf));
    buf[0] = 0;
    buf[1] = 0x40;
    WriteToDevice(refDevice, buf, DEVICE_BUFSIZE);
    memset(buf, 0x00, sizeof(buf));
    ReadFromeDevice(refDevice, buf, DEVICE_BUFSIZE, 0.5);
    
    fprintf(stderr, "waiting...\n");
    
    // デバイスをリモコンデータ受信待ちモードに
    memset(buf, 0xFF, sizeof(buf));
    buf[0] = 0;
    buf[1] = 0x51;
    buf[2] = RECEIVE_WAIT_MODE_WAIT;
    if (WriteToDevice(refDevice, buf, DEVICE_BUFSIZE) != kIOReturnSuccess) {
        fprintf(stderr, "WriteToDevice: err\n");
        goto DONE;
    }
    memset(buf, 0x00, sizeof(buf));
    if (ReadFromeDevice(refDevice, buf, DEVICE_BUFSIZE, 0.5) < 0) {
        fprintf(stderr, "ReadFromeDevice: err\n");
        goto DONE;
    }
    if (buf[1] != 0x51) {
        fprintf(stderr, "invalid response");
        goto DONE;
    }
    // リモコンデータ受信待ち
    while (true) {
        memset(buf, 0xFF, sizeof(buf));
        buf[0] = 0;
        buf[1] = 0x50;
        WriteToDevice(refDevice, buf, DEVICE_BUFSIZE);
        memset(buf, 0x00, sizeof(buf));
        ReadFromeDevice(refDevice, buf, DEVICE_BUFSIZE, 0.5);
        if (buf[1] == 0x50 && buf[2] != 0) {
            // 受信データありなら 16 進表示
            for (i = 0; i < 7; i++) {
                printf("%02X", buf[i+2]);
            }
            putchar('\n');
            break;
        }
    }
    // リモコンデータ受信待ちモードを解除
    memset(buf, 0xFF, sizeof(buf));
    buf[0] = 0;
    buf[1] = 0x51;
    buf[2] = RECEIVE_WAIT_MODE_NONE;
    WriteToDevice(refDevice, buf, DEVICE_BUFSIZE);
    memset(buf, 0x00, sizeof(buf));
    ReadFromeDevice(refDevice, buf, DEVICE_BUFSIZE, 0.5);
    sts = 0;
DONE:
    return sts;
}

IOHIDManagerRef refHidMgr = NULL;
CFSetRef refDevSet = NULL;
IOHIDDeviceRef *prefDevs = NULL;

IOHIDDeviceRef open_device()
{
    int vid, myVID = 0x22ea; // BTO IR REMOCON のベンダ ID
    int pid, myPID = 0x001e; // BTO IR REMOCON のプロダクト ID
    int i, sts = -1;
    IOReturn ret;
    unsigned char buf[65];
    Boolean doDisplay = false;

    IOHIDDeviceRef refDevice;
    CFIndex numDevices;
    
    doDisplay = true;
    
    // HID マネージャリファレンスを生成
    refHidMgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    // すべての HID デバイスを対象とする
    IOHIDManagerSetDeviceMatching(refHidMgr, NULL);
    IOHIDManagerScheduleWithRunLoop(refHidMgr, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    // HID マネージャを開く
    IOHIDManagerOpen(refHidMgr, kIOHIDOptionsTypeNone);
    // マッチしたデバイス群のセットを得る
    refDevSet = IOHIDManagerCopyDevices(refHidMgr);
    numDevices = CFSetGetCount(refDevSet);
    prefDevs = malloc(numDevices * sizeof(IOHIDDeviceRef));
    // セットから値を取得
    CFSetGetValues(refDevSet, (const void **)prefDevs);
    
    // HID デバイス群を走査して BTO IR REMOCON を探す
    for (i = 0; i < numDevices; i++) {
        refDevice = prefDevs[i];
        // VID, PID をチェック
        vid = getIntProperty(refDevice, CFSTR(kIOHIDVendorIDKey)); 
        pid = getIntProperty(refDevice, CFSTR(kIOHIDProductIDKey));
        if (vid != myVID || pid != myPID) {
            refDevice = NULL;
            continue;
        }
        // デバイスのオープン
        ret = IOHIDDeviceOpen(refDevice, kIOHIDOptionsTypeNone);    
        if (ret != kIOReturnSuccess) {
            refDevice = NULL;
            continue;
        }
        // 試し打ち
        memset(buf, 0xFF, sizeof(buf));
        buf[0] = 0x00;
        buf[1] = 0x40;
        if (WriteToDevice(refDevice, buf, DEVICE_BUFSIZE) == kIOReturnSuccess) {
            memset(buf, 0, sizeof(buf));
            int bytes = ReadFromeDevice(refDevice, buf, DEVICE_BUFSIZE, 0.5);
            if (bytes >= 0 && buf[1] == 0x40) {
                break; // OK
            }
        }
        IOHIDDeviceClose(refDevice, kIOHIDOptionsTypeNone);
        refDevice = NULL;
    }
    return refDevice;
}

void close_device(IOHIDDeviceRef refDevice)
{
    IOHIDDeviceClose(refDevice, kIOHIDOptionsTypeNone);
    
    if (prefDevs) {
        free(prefDevs);
    }
    if (refDevSet) {
        CFRelease(refDevSet);
    }
    if (refHidMgr) {
        IOHIDManagerClose(refHidMgr, kIOHIDOptionsTypeNone);
        CFRelease(refHidMgr);    
    }
}
#endif
