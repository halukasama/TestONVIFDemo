#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "soapH.h"
#include "soapStub.h"
#include "wsdd.h"

#include <uuid/uuid.h>

#define ONVIF_USER          "admin"
#define ONVIF_PASSWORD      "bq111111"

// 通过capa 或者probeMatches 获取，此例不再重复
#define SERVICE_ENDPOINT    "http://192.168.0.236/onvif/device_service"
//SOAP_NAMESPACE_OF_tptz

struct soap* NewSoap(struct soap* soap)
{
    soap = soap_new();
    if(NULL == soap ) {
        printf("[%d][%s][Soap New Error]\n", __LINE__, __func__);
        return NULL;
    }

    soap->recv_timeout = 50;
    soap_set_namespaces(soap, namespaces);

    return soap ;
}
char* GetDeviceServices(struct soap* soap, struct _tds__GetServices *getServices, struct _tds__GetServicesResponse *getServicesResponse)
{
    char* mediaEndPoint = "";
    getServices->IncludeCapability = xsd__boolean__false_;

    printf("[%d][%s][---- Getting Device Services ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___tds__GetServices(soap, SERVICE_ENDPOINT, NULL, getServices, getServicesResponse);
    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);

    int i;
    if (getServicesResponse->Service != NULL) {
        for(i = 0; i < getServicesResponse->__sizeService; i++) {
            if (!strcmp(getServicesResponse->Service[i].Namespace, SOAP_NAMESPACE_OF_trt)) {
                mediaEndPoint = getServicesResponse->Service[i].XAddr;
                printf("[%d][%s][MediaServiceAddress:%s]\n",__LINE__, __func__, mediaEndPoint);
                break;
            }
        }
    }
    return mediaEndPoint;
}

int GetProfiles(struct soap* soap, struct _trt__GetProfiles *trt__GetProfiles,
                  struct _trt__GetProfilesResponse *trt__GetProfilesResponse, char* media_ep)
{
    int result=0 ;
    printf("[%d][%s][---- Getting Profiles ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    result = soap_call___trt__GetProfiles(soap, media_ep, NULL, trt__GetProfiles, trt__GetProfilesResponse);

    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);
    if (result == SOAP_EOF) {
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return;
    }
    if(trt__GetProfilesResponse->Profiles != NULL) {
        if(trt__GetProfilesResponse->Profiles->Name != NULL)
            printf("[%d][%s][Profiles Name:%s]\n",__LINE__, __func__, trt__GetProfilesResponse->Profiles->Name);
        if(trt__GetProfilesResponse->Profiles->VideoEncoderConfiguration != NULL)
            printf("[%d][%s][Profiles Token:%s]\n",__LINE__, __func__, trt__GetProfilesResponse->Profiles->VideoEncoderConfiguration->Name);
    }
    return result;
}

void getOsds(struct soap* soap , char* ep, struct _trt__GetProfilesResponse *trt__GetProfilesResponse, struct _trt__GetOSDs *trt__GetOSDs, struct _trt__GetOSDsResponse *trt__GetOSDsResponse)
{
    trt__GetOSDs->ConfigurationToken = trt__GetProfilesResponse->Profiles->VideoSourceConfiguration->token;

    printf("[%d][---- start get osdconfig ----]\n", __LINE__);
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___trt__GetOSDs(soap, ep, NULL, trt__GetOSDs, trt__GetOSDsResponse);
    printf("errorNo: %d\n",result);
#if 0
    if (trt__GetOSDsResponse->OSDs == NULL) {
        if (trt__GetOSDsResponse->OSDs->token != NULL)
            printf("[token]:%s\n", trt__GetOSDsResponse->OSDs->token);
    }
    int i;
    for (i = 0; i < trt__GetOSDsResponse->__sizeOSDs; i++) {
        if (trt__GetOSDsResponse->OSDs->TextString->PlainText != NULL)
            printf ("text:%s\n", trt__GetOSDsResponse->OSDs[i].TextString->PlainText);
            printf ("text:%s\n", trt__GetOSDsResponse->OSDs[i].TextString->PlainText);
            printf ("text:%s\n", trt__GetOSDsResponse->OSDs[i].TextString->PlainText);
    }
    printf("[Extension]size:%d\n", sizeof(trt__GetOSDsResponse->OSDs->Extension));
    printf("[Image]size:%d\n", sizeof(trt__GetOSDsResponse->OSDs->Image));
    printf("[anyAttribute]size:%d\n", sizeof(trt__GetOSDsResponse->OSDs->__anyAttribute));
    printf("[%d][---- get osdconfig compelete----]\n", __LINE__);
#endif
}

void CreateDataTimeOsd(struct soap*soap, char*ep, struct _trt__CreateOSD *req, struct _trt__CreateOSDResponse *resp, struct _trt__GetOSDsResponse *osdcfg)
{
    struct tt__OSDConfiguration osd;
    char* token = "";//"OsdToken_101";
    struct tt__OSDReference reftoken;
    osd.token = token;
    reftoken.__item = "VideoSourceToken";
    reftoken.__anyAttribute = NULL;
    osd.VideoSourceConfigurationToken = &reftoken;
    osd.Type = tt__OSDType__Text;
    osd.__anyAttribute = NULL;

    struct tt__OSDPosConfiguration pos;
    char *postype = "Custom";
    pos.Type = postype;
    float x = -0.95;
    float y = 0.8;
    struct tt__Vector coor;
    coor.x = &x;
    coor.y = &y;
    pos.Pos = &coor;
    struct tt__OSDPosConfigurationExtension posextension;
    posextension.__any = "";
    posextension.__size = 8;
    posextension.__anyAttribute = "";
    pos.__anyAttribute = "";
    pos.Extension = &posextension;
    struct tt__OSDTextConfiguration textstr;
    struct tt__OSDColor fontcolor;
    struct tt__OSDColor backgroundcolor;
    struct tt__OSDTextConfigurationExtension textstrExtension;
    textstrExtension.__any = "";
    textstrExtension.__size = 1;
    textstrExtension.__anyAttribute = "";

    struct tt__Color color;
    //<tt:Color X="16.000000" Y="128.000000" Z="128.000000"
        //Colorspace="http://www.onvif.org/ver10/colorspace/YCbCr"
    color.X = 16;
    color.Y = 128;
    color.Z = 128;
    color.Colorspace = "http://www.onvif.org/ver10/colorspace/YCbCr";
    fontcolor.Color = &color;
    int transparent = 0;
    fontcolor.Transparent = &transparent;
    fontcolor.__anyAttribute = "";

    backgroundcolor.Color = &color;
    backgroundcolor.Transparent = &transparent;
    backgroundcolor.__anyAttribute = "";
    textstr.Extension = &textstrExtension;
    textstr.FontColor = &fontcolor;
    textstr.BackgroundColor = &backgroundcolor;

    char *strtype = "DateAndTime";
    int fontsize = 64;
    int *FontSize = &fontsize;
    char *datefmt = "yyyy/MM/dd";
    char *timefmt = "HH:mm:ss";
    textstr.Type = strtype;
    textstr.FontSize = FontSize;
    textstr.DateFormat = datefmt;
    textstr.TimeFormat = timefmt;
    textstr.__anyAttribute = "";
    osd.Extension = NULL;// osdcfg->OSDs->Extension;
    osd.Image = NULL;//osdcfg->OSDs->Image;
    osd.TextString = &textstr;
    osd.Position = &pos;

    struct tt__OSDConfigurationExtension extension;
    extension.__any = "";
    extension.__size = 0;
    extension.__anyAttribute = "";
    osd.Extension = &extension;
    req->OSD = &osd;
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    soap_call___trt__CreateOSD(soap, ep, NULL, req, resp);
    printf("float_format ????  %   \n", soap->float_format);
}

void deleteDatetimeOsd(struct soap* soap, char* ep, struct _trt__DeleteOSD *req, struct _trt__DeleteOSDResponse *resp)
{
    char *token = "OsdToken_101";
    req->OSDToken = token;
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    soap_call___trt__DeleteOSD(soap, ep, NULL, req, resp);
}
void CreateTextOsd(struct soap*soap, char*ep, struct _trt__CreateOSD *req, struct _trt__CreateOSDResponse *resp, struct _trt__GetOSDsResponse *osdcfg)
{
    struct tt__OSDConfiguration osd;
    char* token = "";
    osd.token = token;
    osd.VideoSourceConfigurationToken = osdcfg->OSDs->VideoSourceConfigurationToken;
    osd.Type = osdcfg->OSDs->Type;
    struct tt__OSDPosConfiguration pos;
    char *postype = "Custom";
    pos.Type = postype;
    float x = 0.10;
    float y = -0.75;
    struct tt__Vector coor;
    coor.x = &x;
    coor.y = &y;
    pos.Pos = &coor;
    struct tt__OSDPosConfigurationExtension posextension;
    posextension.__any = "";
    posextension.__size = 8;
    posextension.__anyAttribute = "";
    pos.__anyAttribute = "";
    pos.Extension = &posextension;
    struct tt__OSDTextConfiguration textstr;
    struct tt__OSDColor fontcolor;
    struct tt__OSDColor backgroundcolor;
    struct tt__OSDTextConfigurationExtension textstrExtension;
    textstrExtension.__any = "";
    textstrExtension.__size = 1;
    textstrExtension.__anyAttribute = "";

    struct tt__Color color;
    //<tt:Color X="16.000000" Y="128.000000" Z="128.000000"
        //Colorspace="http://www.onvif.org/ver10/colorspace/YCbCr"
    color.X = 16;
    color.Y = 128;
    color.Z = 128;
    color.Colorspace = "http://www.onvif.org/ver10/colorspace/YCbCr";
    fontcolor.Color = &color;
    int transparent = 0;
    fontcolor.Transparent = &transparent;
    fontcolor.__anyAttribute = "";

    backgroundcolor.Color = &color;
    backgroundcolor.Transparent = &transparent;
    backgroundcolor.__anyAttribute = "";
    textstr.Extension = &textstrExtension;
    textstr.FontColor = &fontcolor;
    textstr.BackgroundColor = &backgroundcolor;

    char *strtype = "Plain";
    int fontsize = 64;
    int *FontSize = &fontsize;
    char *plaintext = "支持中文字体";
    textstr.Type = strtype;
    textstr.FontSize = FontSize;
    textstr.PlainText = plaintext;
    textstr.TimeFormat = "";
    textstr.DateFormat = "";
    textstr.__anyAttribute = "";
    osd.Extension = NULL;
    osd.Image = NULL;
    osd.TextString = &textstr;
    osd.Position = &pos;

    struct tt__OSDConfigurationExtension extension;
    extension.__any = "";
    extension.__size = 0;
    extension.__anyAttribute = "";
    osd.Extension = &extension;
    req->OSD = &osd;

    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    soap_call___trt__CreateOSD(soap, ep, NULL, req, resp);
    printf("float_format ????  %   \n", soap->float_format);
}

void deleteTextOsd(struct soap* soap, char*ep, struct _trt__DeleteOSD*req, struct _trt__DeleteOSDResponse *resp, struct _trt__GetOSDsResponse *osdcfg)
{
    char *token = "OsdToken_102";
    int i;
    for (i = 0; i <= osdcfg->__sizeOSDs; i++) {
            req->OSDToken = token;
            soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
            soap_call___trt__DeleteOSD(soap, ep, NULL, req, resp);
        }
}
#if 0
void GetOsd()
{
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    soap_call___trt__GetOSD(soap, ep, NULL, req, resp);
}
#endif
void SetOsd(struct soap*soap, char*ep, struct _trt__SetOSD *req, struct _trt__SetOSDResponse *resp, struct _trt__GetOSDsResponse *osdcfg)
{
    struct tt__OSDConfiguration osd;
    char *text = "支持中文abc";
    osdcfg->OSDs[1].TextString->PlainText = text;
#if 0
    int i;
    for (i = 0; i <= osdcfg->__sizeOSDs; i++) {
        if (!strcmp(osdcfg->OSDs[i].token,token)) {
            printf("it's plaintext");
            osdcfg->OSDs->TextString->PlainText = text;
            printf("val ready");
        }
    }
#endif
    osd = osdcfg->OSDs[1];
    req->OSD = &osd;
    printf("ready to send request");
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    soap_call___trt__SetOSD(soap, ep, NULL, req, resp);
}

void FollowStepsToPerform(struct soap *soap)
{

    soap_set_mode(soap, SOAP_C_UTFSTRING);
    struct _trt__GetProfiles trt__GetProfiles;
    struct _trt__GetProfilesResponse profiles;

    struct _tds__GetServices getServices;
    soap_default__tds__GetServices(soap, &getServices);
    struct _tds__GetServicesResponse getServicesResponse;

    struct _trt__GetOSDs trt__GetOSDs;
    struct _trt__GetOSDsResponse trt__GetOSDsResponse;

    struct _trt__SetOSD trt__SetOSD;
    struct _trt__SetOSDResponse trt__SetOSDResponse;

    struct _trt__CreateOSD textOSD;
    struct _trt__CreateOSD datatimeOSD;
    struct _trt__CreateOSDResponse textOSDresp;
    struct _trt__CreateOSDResponse datatimeOSDresp;

    struct _trt__DeleteOSD delDatetimeOsd;
    struct _trt__DeleteOSD delChannlOsd;

    struct _trt__DeleteOSDResponse delDatetimeOsdresp;
    struct _trt__DeleteOSDResponse delChannlOsdresp;

    char *media_ep = GetDeviceServices(soap, &getServices, &getServicesResponse);
    if (media_ep != "") {
        GetProfiles(soap, &trt__GetProfiles, &profiles, media_ep);
        getOsds(soap, media_ep, &profiles, &trt__GetOSDs, &trt__GetOSDsResponse);
//        deleteDatetimeOsd(soap, media_ep, &delDatetimeOsd, &delDatetimeOsdresp);
        CreateDataTimeOsd(soap, media_ep, &datatimeOSD, &datatimeOSDresp, &trt__GetOSDsResponse);
//        getOsds(soap, media_ep, &profiles, &trt__GetOSDs, &trt__GetOSDsResponse);
//        deleteTextOsd(soap, media_ep, &delChannlOsd, &delChannlOsdresp, &trt__GetOSDsResponse);
        CreateTextOsd(soap, media_ep, &textOSD, &textOSDresp, &trt__GetOSDsResponse);
//        SetOsd(soap, media_ep, &trt__SetOSD, &trt__SetOSDResponse, &trt__GetOSDsResponse);
    }
}

int main()
{
    printf("[%s][%d][%s][%s][---- ONVIF Device Test Started ----]\n", __FILE__, __LINE__, __TIME__, __func__);
    struct soap *soap;
    soap = NewSoap(soap);

    FollowStepsToPerform(soap);

    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);

    printf("[%s][%d][%s][%s][---- ONVIF Device Test Complete ----]\n", __FILE__, __LINE__, __TIME__, __func__);
    return 0;
}
