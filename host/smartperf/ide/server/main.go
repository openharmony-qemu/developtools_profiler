// Copyright (C) 2022 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package main

//遇到报错请在当前目录下执行这个命令： go mod download golang.org/x/text
import (
    "bufio"
    "bytes"
    "crypto/rand"
    "crypto/rsa"
    "crypto/tls"
    "crypto/x509"
    "crypto/x509/pkix"
    "encoding/json"
    "encoding/pem"
    "fmt"
    "io"
    "io/fs"
    "log"
    "math/big"
    "mime"
    "net"
    "net/http"
    "net/http/cookiejar"
    "os"
    "os/exec"
    "path"
    "path/filepath"
    "regexp"
    "runtime"
    "strconv"
    "strings"
    "time"
)

const HttpPort = 9000

var exPath string
var serveInfo string

// CGO_ENABLED=0 GOOS=windows GOARCH=amd64 go build main.go
// CGO_ENABLED=0 GOOS=darwin GOARCH=amd64 go build main.go
func cors(fs http.Handler, version string) http.HandlerFunc {
    return func(w http.ResponseWriter, r *http.Request) {
        // return if you do not want the FileServer handle a specific request
        r.Header.Add("Cross-Origin-Opener-Policy", "same-origin")
        r.Header.Add("Cross-Origin-Embedder-Policy", "require-corp")
        w.Header().Add("Cross-Origin-Opener-Policy", "same-origin")
        w.Header().Add("Cross-Origin-Embedder-Policy", "require-corp")
        w.Header().Set("Access-Control-Allow-Origin", "*")
        w.Header().Set("Access-Control-Allow-Credentials", "true")
        w.Header().Set("Access-Control-Allow-Headers", "x-requested-with, authorization, blade-auth") //*
        w.Header().Set("Access-Control-Allow-Methods", "*")                                           //*
        w.Header().Set("Access-Control-Max-Age", "3600")
        w.Header().Set("data-version", version)
        fs.ServeHTTP(w, r)
    }
}

func exist(path string) bool {
    _, err := os.Stat(path)
    if err != nil {
        if os.IsExist(err) {
            return true
        }
        return false
    }
    return true
}
func genSSL() {
    if exist("cert/keyFile.key") || exist("cert/certFile.pem") {
        fmt.Println("keyFile.key exists")
        return
    }
    max := new(big.Int).Lsh(big.NewInt(1), 128)
    serialNumber, _ := rand.Int(rand.Reader, max)
    subject := pkix.Name{
        Organization:       []string{"www.smartperf.com"},
        OrganizationalUnit: []string{"ITs"},
        CommonName:         "www.smartperf.com",
    }
    certificate509 := x509.Certificate{
        SerialNumber: serialNumber,
        Subject:      subject,
        NotBefore:    time.Now(),
        NotAfter:     time.Now().AddDate(10, 0, 0),
        KeyUsage:     x509.KeyUsageKeyEncipherment | x509.KeyUsageDigitalSignature,
        ExtKeyUsage:  []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth},
        IPAddresses:  []net.IP{net.ParseIP("127.0.0.1")},
    }
    chekDir("cert")
    pk, _ := rsa.GenerateKey(rand.Reader, 1024)
    derBytes, _ := x509.CreateCertificate(rand.Reader, &certificate509, &certificate509, &pk.PublicKey, pk)
    certOut, _ := os.Create("cert/certFile.pem")
    pem.Encode(certOut, &pem.Block{Type: "CERTIFICATE", Bytes: derBytes})
    certOut.Close()
    keyOut, _ := os.Create("cert/keyFile.key")
    pem.Encode(keyOut, &pem.Block{Type: "RSA PRIVATE KEY", Bytes: x509.MarshalPKCS1PrivateKey(pk)})
    keyOut.Close()
}
func main() {
    checkPort(HttpPort)
    genSSL()
    exPath = getCurrentAbPath()
    fmt.Println(exPath)
    go func() {
        version := ""
        readVersion, versionErr := os.ReadFile(exPath + "/version.txt")
        if versionErr != nil {
            version = ""
        } else {
            version = string(readVersion)
        }
        readReqServerConfig()
        mux := http.NewServeMux()
        mime.TypeByExtension(".js")
        mime.AddExtensionType(".js", "application/javascript")
        log.Println(mime.TypeByExtension(".js"))
        mux.HandleFunc("/logger", consoleHandler)
        mux.Handle("/application/upload/", http.StripPrefix("/upload/", http.FileServer(http.Dir(filepath.FromSlash(exPath+"/upload")))))
        mux.HandleFunc("/application//download-file", downloadHandler)
        mux.HandleFunc("/application/serverInfo", serverInfo)
        fs := http.FileServer(http.Dir(exPath + "/"))
        mux.Handle("/application/", http.StripPrefix("/application/", cors(fs, version)))
        go func() {
            ser := &http.Server{
                Addr:    fmt.Sprintf(":%d", HttpPort),
                Handler: mux,
            }
            log.Println(fmt.Sprintf("HTTPS[%d]服务启动", HttpPort))
            err := ser.ListenAndServeTLS("cert/certFile.pem", "cert/keyFile.key")
            CheckErr(err)
        }()
        go func() {
            ser := &http.Server{
                Addr:    fmt.Sprintf(":%d", HttpPort+1),
                Handler: mux,
            }
            log.Println(fmt.Sprintf("HTTP[%d]服务启动", HttpPort))
            err := ser.ListenAndServe()
            CheckErr(err)
        }()
        open(fmt.Sprintf("https://localhost:%d/application", HttpPort))
    }()
    select {}
}

func getPidByPort(portNumber int) int {
    resPid := -1
    var out bytes.Buffer
    cmdRes := exec.Command("cmd", "/c", fmt.Sprintf("netstat -ano -p tcp | findstr %d", portNumber))
    cmdRes.Stdout = &out
    cmdRes.Run()
    cmdResStr := out.String()
    findStr := regexp.MustCompile(`\s\d+\s`).FindAllString(cmdResStr, -1)
    if len(findStr) > 0 {
        pid, err := strconv.Atoi(strings.TrimSpace(findStr[0]))
        if err != nil {
            resPid = -1
        } else {
            resPid = pid
        }
    }
    return resPid
}

type LoggerReq struct {
    FileName string `json:"fileName"`
    FileSize string `json:"fileSize"`
}

func consoleHandler(w http.ResponseWriter, r *http.Request) {
    chekDir(exPath + "/logger")
    var now = time.Now()
    var fileName = fmt.Sprintf("%d-%d-%d", now.Year(), now.Month(), now.Day())
    dst, err := os.OpenFile(exPath+"/logger/"+fileName, os.O_WRONLY|os.O_CREATE|os.O_APPEND|os.O_SYNC, 0666)
    CheckErr(err)
    contentType := r.Header["Content-Type"]
    if len(contentType) > 0 {
        contentTypeName := contentType[0]
        if strings.HasPrefix(contentTypeName, "application/json") {
            decoder := json.NewDecoder(r.Body)
            var req LoggerReq
            decoder.Decode(&req)
            dst.WriteString(fmt.Sprintf("%s %s (%s M)\n", now.Format("2006-01-02 15:04:05"), req.FileName, req.FileSize))
            fmt.Fprintf(w, fmt.Sprintf("日志写入成功%s", exPath))
        }
    }
}

func serverInfo(w http.ResponseWriter, r *http.Request) {
  w.Header().Set("Access-Control-Allow-Origin", "*")
  w.Header().Set("request_info", serveInfo)
  w.WriteHeader(200)
}

func readReqServerConfig() string {
  readServerConfig, serverConfigErr := os.ReadFile(exPath + "/server-config.txt")
  if serverConfigErr != nil {
      serveInfo = ""
  } else {
      serveInfo = string(readServerConfig)
  }
  return serveInfo
}

func mapToJson(m map[string]interface{}) (string, error) {
    marshal, err := json.Marshal(m)
    if err != nil {
        return "", err
    }
    var str = string(marshal)
    return str, nil
}
func jsonToMap(str string) (map[string]interface{}, error) {
    var m = make(map[string]interface{})
    err := json.Unmarshal([]byte(str), &m)
    if err != nil {
        return nil, err
    }
    return m, nil
}

// MkDir 创建目录
func MkDir(path string) {
    dir := path[0:strings.LastIndex(path, string(os.PathSeparator))] //从文件路径获取目录
    if _, err := os.Stat(dir); err != nil {                          //如果目录不存在，创建目录
        os.MkdirAll(dir, os.ModePerm)
    }
}

func resp(w *http.ResponseWriter) func(bool, int, string, map[string]interface{}) {
    return func(success bool, code int, msg string, obj map[string]interface{}) {
        toJson, err := mapToJson(map[string]interface{}{
            "success": success,
            "code":    code,
            "msg":     msg,
            "data":    obj,
        })
        if err != nil {
            errRes, _ := mapToJson(map[string]interface{}{
                "success": false,
                "code":    -1,
                "msg":     err.Error(),
            })
            fmt.Fprintf(*w, errRes)
        } else {
            fmt.Fprintf(*w, toJson)
        }
    }
}

func get(url string) (*http.Response, error) {
    jar, _ := cookiejar.New(nil)
    c := &http.Client{
        Transport:     &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}},
        CheckRedirect: nil,
        Jar:           jar,
        Timeout:       time.Duration(3600) * time.Second,
    }
    return c.Get(url)
}

func clearOverdueFile() {
    MkDir(filepath.FromSlash(fmt.Sprintf("./upload/")))
    now := time.Now()
    loc, err := time.LoadLocation("Asia/Shanghai")
    if err != nil {
        return
    }
    var checkDue = func(fileName string) bool {
        f := getSuffixByUrl(fileName)
        parseTime, err := time.ParseInLocation("20060102150405000", f.fileName, loc)
        if err != nil {
            return false
        }
        sub := now.Sub(parseTime)
        if sub.Minutes() > 60 { //bigger than 60 min flag due
            return true
        }
        return false
    }
    slash := filepath.FromSlash(fmt.Sprintf("./upload/"))
    filepath.WalkDir(slash, func(path string, d fs.DirEntry, err error) error {
        if checkDue(d.Name()) {
            fmt.Println(now, "delete->", path, d.Name(), err)
            os.Remove(path)
        }
        return nil
    })
}
func getSuffixByUrl(u string) struct {
    fileName string
    suffix   string
} {
    lastIndex := strings.LastIndex(u, "/")
    var f string
    if lastIndex != -1 {
        f = u[lastIndex:]
    } else {
        f = u
    }
    index := strings.LastIndex(f, ".")
    if index != -1 {
        return struct {
            fileName string
            suffix   string
        }{
            f[0:index],
            f[index:],
        }
    } else {
        return struct {
            fileName string
            suffix   string
        }{
            f,
            "",
        }
    }
}

func downloadHandler(w http.ResponseWriter, r *http.Request) {
    w.Header().Set("content-type", "text/json")
    clearOverdueFile()
    contentType := r.Header["Content-Type"]
    if len(contentType) > 0 {
        contentTypeName := contentType[0]
        if strings.HasPrefix(contentTypeName, "application/x-www-form-urlencoded") {
            url := r.PostFormValue("url")
            res, err := get(url)
            if err != nil {
                resp(&w)(false, -1, err.Error(), nil)
                return
            }
            pth := filepath.FromSlash(fmt.Sprintf("/upload/%s%s", time.Now().Format("20060102150405000"), getSuffixByUrl(url).suffix))
            MkDir("." + pth)
            create, err := os.Create("." + pth)
            if err != nil {
                resp(&w)(false, -1, err.Error(), nil)
                return
            }
            written, err := io.Copy(create, res.Body)
            if err != nil {
                resp(&w)(false, -1, err.Error(), nil)
                return
            }
            fmt.Println(url, written)
            pth = "/application" + pth
            resp(&w)(true, 0, "success", map[string]interface{}{
                "url":  pth,
                "size": written,
            })
            return
        }
    }
    resp(&w)(false, -1, "请求方式错误", nil)
}

func SplitLines(s string) []string {
    var lines []string
    sc := bufio.NewScanner(strings.NewReader(s))
    for sc.Scan() {
        lines = append(lines, sc.Text())
    }
    return lines
}

func readFileFirstLine(path string) string {
    file, err := os.Open(path)
    if err != nil {
        return ""
    }
    defer file.Close()

    readFile := bufio.NewReader(file)
    line, readErr := readFile.ReadString('\n')
    if readErr != nil || io.EOF == err {
        return ""
    }
    return line
}

func PathExists(path string) (bool, error) {
    _, err := os.Stat(path)
    if err == nil {
        return true, nil
    }
    if os.IsNotExist(err) {
        return false, nil
    }
    return false, err
}

func chekDir(path string) {
    _, err := os.Stat(path)
    if err != nil {
        err := os.Mkdir(path, os.ModePerm)
        if err != nil {
            fmt.Printf("mkdir failed![%v]\n", err)
        } else {
            fmt.Printf("mkdir success!\n")
        }
    }
}
func CheckErr(err error) {
    if err != nil {
        log.Panicln(err)
    }
}

func open(url string) error {
    if isWindows() {
        return openUrlWindows(url)
    } else if isDarwin() {
        return openUrlDarwin(url)
    } else {
        return openUrlOther(url)
    }
}

func openUrlWindows(url string) error {
    cmd := "cmd"
    args := []string{"/c", "start", url}
    return exec.Command(cmd, args...).Start()
}
func openUrlDarwin(url string) error {
    var cmd = "open"
    var args = []string{url}
    return exec.Command(cmd, args...).Start()
}
func openUrlOther(url string) error {
    var cmd = "xdg-open"
    var args = []string{url}
    return exec.Command(cmd, args...).Start()
}

func isWindows() bool {
    return runtime.GOOS == "windows"
}
func isDarwin() bool {
    return runtime.GOOS == "darwin"
}

func getCurrentAbPath() string {
    dir := getExecutePath()
    tmpDir, _ := filepath.EvalSymlinks(os.TempDir())
    if strings.Contains(dir, tmpDir) {
        return getCallerPath()
    }
    return dir
}

func getCallerPath() string {
    var pth string
    _, fName, _, ok := runtime.Caller(0)
    if ok {
        pth = path.Dir(fName)
    }
    return pth
}
func getExecutePath() string {
    pth, err := os.Executable()
    if err != nil {
        log.Fatal(err)
    }
    res, _ := filepath.EvalSymlinks(filepath.Dir(pth))
    return res
}

func checkPort(port int) {
    if isWindows() {
        pid := getPidByPort(port)
        if pid != -1 {
            res := exec.Command("cmd", "/c", fmt.Sprintf("taskkill /F /PID %d /T", pid))
            res.Run()
        }
    }
}
