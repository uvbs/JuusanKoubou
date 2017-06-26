package downloader

import (
    . "ml/strings"
)

type Downloader interface {
    Analysis() (AnalysisResult, error)
    Download(path String) (DownloadResult, error)
    Close()
}
