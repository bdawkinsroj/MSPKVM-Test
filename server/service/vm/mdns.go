package vm

import (
	"MSPKVM-Server/proto"
	"fmt"
	"os"
	"os/exec"
	"strings"

	"github.com/gin-gonic/gin"
	log "github.com/sirupsen/logrus"
)

const (
	AvahiDaemonPid          = "/run/avahi-daemon/pid"
	AvahiDaemonScript       = "/etc/init.d/S50avahi-daemon"
	AvahiDaemonBackupScript = "/kvmapp/system/init.d/S50avahi-daemon"
)

func (s *Service) GetMdnsState(c *gin.Context) {
	var rsp proto.Response

	pid := getAvahiDaemonPid()

	rsp.OkRspWithData(c, &proto.GetMdnsStateRsp{
		Enabled: pid != "",
	})
}

func (s *Service) EnableMdns(c *gin.Context) {
	var rsp proto.Response

	pid := getAvahiDaemonPid()
	if pid != "" {
		rsp.OkRsp(c)
		return
	}

	commands := []string{
		fmt.Sprintf("cp -f %s %s", AvahiDaemonBackupScript, AvahiDaemonScript),
		fmt.Sprintf("%s start", AvahiDaemonScript),
	}

	command := strings.Join(commands, " && ")
	err := exec.Command("sh", "-c", command).Run()
	if err != nil {
		log.Errorf("failed to start avahi-daemon: %s", err)
		rsp.ErrRsp(c, -1, "failed to enable mdns")
		return
	}

	rsp.OkRsp(c)
	log.Debugf("avahi-daemon started")
}

func (s *Service) DisableMdns(c *gin.Context) {
	var rsp proto.Response

	pid := getAvahiDaemonPid()
	if pid == "" {
		rsp.OkRsp(c)
		return
	}

	command := fmt.Sprintf("kill -9 %s", pid)
	err := exec.Command("sh", "-c", command).Run()
	if err != nil {
		log.Errorf("failed to stop avahi-daemon: %s", err)
		rsp.ErrRsp(c, -1, "failed to disable mdns")
		return
	}

	_ = os.Remove(AvahiDaemonPid)
	_ = os.Remove(AvahiDaemonScript)

	rsp.OkRsp(c)
	log.Debugf("avahi-daemon stopped")
}

func getAvahiDaemonPid() string {
	if _, err := os.Stat(AvahiDaemonPid); err != nil {
		return ""
	}

	content, err := os.ReadFile(AvahiDaemonPid)
	if err != nil {
		log.Errorf("failed to read mdns pid: %s", err)
		return ""
	}

	return strings.ReplaceAll(string(content), "\n", "")
}
