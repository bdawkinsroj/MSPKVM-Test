package vm

import (
	"MSPKVM-Server/common"
	"MSPKVM-Server/proto"
	"time"

	"github.com/gin-gonic/gin"
	log "github.com/sirupsen/logrus"
)

func (s *Service) ResetHdmi(c *gin.Context) {
	var rsp proto.Response

	vision := common.GetKvmVision()

	vision.SetHDMI(false)
	time.Sleep(1 * time.Second)
	vision.SetHDMI(true)

	rsp.OkRsp(c)
	log.Debug("reset hdmi")
}
