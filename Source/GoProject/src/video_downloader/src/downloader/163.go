package downloader

import (
    . "ml/strings"
    . "ml/dict"

    "os"
    "fmt"
    "regexp"
    "crypto/aes"
    "encoding/hex"
    xmllib "encoding/xml"

    "ml/net/http"
    "ml/crypto/cipher"
    "ml/encoding/xml"
)

type NetEaseDownloader struct {
    *baseDownloader
    subtitle String
}

func NewNetEase(url String) Downloader {
    d := &NetEaseDownloader{
        baseDownloader: newBase(url),
    }

    return d
}

func (self *NetEaseDownloader) Analysis() (result AnalysisResult, err error) {
    result = AnalysisNotSupported

    resp, err := self.session.Get(self.url)
    if err != nil {
        return
    }

    content := resp.Text()

    pattern := regexp.MustCompile(`(?sU)getCurrentMovie.*\s+src\s*:\s*'(.*)',`)
    swfUrl := String(pattern.FindStringSubmatch(content.String())[1])

    pattern = regexp.MustCompile(`(?sU)getCurrentMovie.*\s+title\s*:\s*'(.*)',`)
    self.title = String(pattern.FindStringSubmatch(content.String())[1])

    vid := swfUrl.Split("-")[2]
    xmlurl := fmt.Sprintf("http://live.ws.126.net/movie/%s/%s/%s.xml", vid.SubString(len(vid) - 2, len(vid) - 1), vid.SubString(len(vid) - 1, len(vid)), vid)

    resp, err = self.session.Get(
        xmlurl,
        http.Headers(Dict{
            "X-Requested-With" : "ShockwaveFlash/23.0.0.166",
        }),
    )

    if err != nil {
        return
    }

    type FlvURL struct {
        HD struct {
            Default     bool            `xml:"default,attr"`
            FLV         []String        `xml:"flv"`
        } `xml:"hd"`
    }

    type PlayURL struct {
        SHD struct {
            Default     bool            `xml:"default,attr"`
            MP4         []String        `xml:"mp4"`
        } `xml:"SHD"`

        HD struct {
            Default     bool            `xml:"default,attr"`
            MP4         []String        `xml:"mp4"`
        } `xml:"HD"`

        SD struct {
            Default     bool            `xml:"default,attr"`
            MP4         []String        `xml:"mp4"`
        } `xml:"SD"`
    }

    type SubtitleURL struct {
        Name            String        `xml:"name"`
        Url             String        `xml:"url"`
    }

    type VideoInfo struct {
        XMLName         xmllib.Name     `xml:"all"`
        Title           String          `xml:"title"`
        PNumber         int             `xml:"pnumber"`
        Encrypt         int             `xml:"encrypt"`

        FlvUrl          FlvURL          `xml:"flvUrl"`
        FlvUrlOrigin    FlvURL          `xml:"flvUrlOrigin"`
        PlayUrl         PlayURL         `xml:"playurl"`
        PlayUrlOrigin   PlayURL         `xml:"playurl_origin"`
        Subtitles       []SubtitleURL   `xml:"subs>sub"`
    }

    var info VideoInfo

    err = xml.Unmarshal(resp.Content, &info)
    if err != nil {
        return
    }

    links := []String{}
    encryptedLinks := [][]String{
        info.FlvUrlOrigin.HD.FLV,
        info.PlayUrlOrigin.SD.MP4,
        info.FlvUrl.HD.FLV,
        info.PlayUrl.SHD.MP4,
        info.PlayUrl.HD.MP4,
        info.PlayUrl.SD.MP4,
    }

    for _, l := range encryptedLinks {
        for index, x := range l {
            encrypted, _ := hex.DecodeString(x.String())
            l[index] = self.decryptoVideoUrl(encrypted, info.Encrypt)
            if len(l[index]) == 0 {
                continue
            }

            links = append(links, l[index])
        }
    }

    //fmt.Printf("%+v\n", spew.Sdump(info))

    for _, sub := range info.Subtitles {
        if sub.Name == "中文" {
            self.subtitle = sub.Url
        }
    }

    self.title = info.Title + " " + self.title
    self.links = append(self.links, DownloadLink{
        url     : links[0],
        name    : self.title + ".flv",
    })

    return AnalysisSuccess, nil
}

func (self *NetEaseDownloader) Download(path String) (result DownloadResult, err error) {
    err = self.downloadSubtitle(path)
    if err != nil {
        return
    }

    return self.baseDownloader.Download(path)
}

func (self *NetEaseDownloader) downloadSubtitle(path String) error {
    if self.subtitle.IsEmpty() == false {
        fmt.Println("\ndownload subtitle")

        resp, err := self.session.Get(
            self.subtitle,
            http.Headers(Dict{
                "X-Requested-With" : "ShockwaveFlash/23.0.0.166",
            }),
        )

        if err != nil {
            fmt.Printf("download subtitle failed: %v", err)
            return err
        }

        p := self.makeFullPath(self.title.String() + ".srt", path.String(), self.title.String())
        fd, err := os.Create(p.String())
        if err != nil {
            fmt.Println(err)
            return err
        }

        fd.Write(resp.Content)
        fd.Close()
    }

    return nil
}

func (self *NetEaseDownloader) decryptoVideoUrl(encrypted []byte, version int) String {
    key := []string{
        "4fxGZqoGmesXqg2o",     // 1
        "3fxVNqoPmesAqg2o",     // 2
    }[version - 1]

    c, _ := aes.NewCipher([]byte(key))
    dec := cipher.NewECBDecrypter(c)

    data := make([]byte, len(encrypted))
    copy(data, encrypted)

    dec.CryptBlocks(data, data)

    return String(data[:len(data) - int(data[len(data) - 1])])
}
