package downloader

import (
    . "ml/strings"
    . "ml/dict"

    "fmt"

    "ml/html"
    "ml/net/http"
    "ml/encoding/json"
    "time"
)

type AcFunDownloader struct {
    *YoukuDownloader
}

func NewAcFun(url String) Downloader {
    d := &AcFunDownloader{
        YoukuDownloader: NewYouku(url).(*YoukuDownloader),
    }

    return d
}

func (self *AcFunDownloader) Analysis() (result AnalysisResult, err error) {
    result = AnalysisNotSupported

    resp, err := self.session.Get(self.url)
    if err != nil {
        return
    }

    doc, err := html.Parse(resp.Text())
    if err != nil {
        return
    }

    pageInfo := String(doc.Find("div#main.main").Find("script").Eq(0).Text())
    pageInfo = pageInfo.Replace("var pageInfo =", "").Trim()

    var ret JsonDict

    err = json.Unmarshal([]byte(pageInfo), &ret)
    if err != nil {
        return
    }

    self.title = ret.Str("title")
    fmt.Println(self.title)

    resp, err = self.session.Get(fmt.Sprintf("http://www.acfun.cn/video/getVideo.aspx?id=%d", ret.Int("videoId")))
    if err != nil {
        return
    }

    err = resp.Json(&ret)
    if err != nil {
        return
    }

    switch sourceType := ret.Str("sourceType"); sourceType {
        case "zhuzhan":
            err = self.youku(ret)

        default:
            fmt.Printf("source type unsupported: %s", sourceType)
            return AnalysisNotSupported, nil
    }

    if err != nil {
        return AnalysisFailed, err
    }

    return AnalysisSuccess, nil
}

func (self *AcFunDownloader) youku(videoInfo JsonDict) error {
    sourceId := videoInfo.Str("sourceId")
    encode := videoInfo.Str("encode")

    resp, err := self.session.Get(
        "https://api.youku.com/players/custom.json",
        http.Params(Dict{
            "client_id" : "908a519d032263f8",
            "video_id"  : sourceId,
            "refer"     : "http://cdn.aixifan.com/player/sslhomura/AcFunV3Player161226.swf",
            "vext"      : "null",
            "embsig"    : encode,
            "styleid"   : "undefined",
            "type"      : "flash",
        }),
    )

    if err != nil {
        return err
    }

    err = resp.Json(&videoInfo)
    if err != nil {
        return err
    }

    playsign := videoInfo.Str("playsign")

    resp, err = self.session.Get(
        "http://aauth-vod.cn-beijing.aliyuncs.com/acfun/web",
        http.Params(Dict{
            "vid"   : sourceId,
            "ct"    : "85",
            "sign"  : playsign,
            "time"  : time.Now().Unix(),
            //"time"  : "1483370889155",
        }),
    )

    if err != nil {
        return err
    }

    err = resp.Json(&videoInfo)
    if err != nil {
        return err
    }

    err = self.parseVideoSegs(videoInfo)
    if err != nil {
        return err
    }

    //self.videoInfo.security.sid, self.videoInfo.security.token = self.decryptSidAndToken(self.videoInfo.security.encryptString)

    return self.parseVideoInfo()
}
