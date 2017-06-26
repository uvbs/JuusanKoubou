package downloader

import (
    . "ml/strings"
    . "ml/dict"

    "fmt"
    "regexp"

    "ml/html"
    "ml/encoding/json"
)

var videoInfoPattern = regexp.MustCompile(`VideoInfo\s*:\s*{.*data\s*:\s*({.*})\s*}.*ObjectInfo\s*:\s*{`)
var locationHrefPattern = regexp.MustCompile(`location\.href\s*=\s*'(.*)';`)

type Ku6Downloader struct {
    *baseDownloader
}

func NewKu6(url String) Downloader {
    d := &Ku6Downloader{
        baseDownloader: newBase(url),
    }

    return d
}

func (self *Ku6Downloader) Analysis() (result AnalysisResult, err error) {
    result = AnalysisNotSupported

    resp, err := self.session.Get(self.url)
    if err != nil {
        return
    }

    content := resp.Text()
    doc, err := html.Parse(content)
    if err != nil {
        return
    }

    if innerFrame := doc.Find("iframe#innerFrame"); innerFrame.Length() != 0 {
        src := innerFrame.MustAttr("src")
        resp, err = self.session.Get(src)
        if err != nil {
            return
        }

        content = resp.Text()

        url := locationHrefPattern.FindStringSubmatch(content.String())[1]
        resp, err = self.session.Get(url)
        if err != nil {
            return
        }

        content = resp.Text()
        doc, err = html.Parse(content)
        if err != nil {
            return
        }
    }

    self.title = String(doc.Find("div.ckl_main").Find("h1").MustAttr("title"))

    videoInfoData := videoInfoPattern.FindStringSubmatch(content.String())[1]

    var data JsonDict
    err = json.LoadData([]byte(videoInfoData), &data)
    if err != nil {
        return
    }

    links := data.Map("data").Str("f").Split(",")

    if len(links) == 1 {
        self.links = append(self.links, DownloadLink{
            url : links[0],
            name: String(fmt.Sprintf("%s.flv", self.title)),
        })
    } else {
        for index, url := range links {
            self.links = append(self.links, DownloadLink{
                url : url,
                name: String(fmt.Sprintf("%s.part%02d.flv", self.title, index + 1)),
            })
        }
    }

    return AnalysisSuccess, nil
}
