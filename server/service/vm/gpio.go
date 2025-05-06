package vm

import (
	"fmt"
	"os"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
	log "github.com/sirupsen/logrus"

	"MSPKVM-Server/config"
	"MSPKVM-Server/proto"
)

func (s *Service) SetGpio(c *gin.Context) {
	var req proto.SetGpioReq
	var rsp proto.Response

	if err := proto.ParseFormRequest(c, &req); err != nil {
		rsp.ErrRsp(c, -1, fmt.Sprintf("invalid arguments: %s", err))
		return
	}

	device := ""
	conf := config.GetInstance().Hardware

	switch req.Type {
	case "power":
		device = conf.GPIOPower
	case "reset":
		device = conf.GPIOReset
	default:
		rsp.ErrRsp(c, -2, fmt.Sprintf("invalid power event: %s", req.Type))
		return
	}

	var duration time.Duration
	if req.Duration > 0 {
		duration = time.Duration(req.Duration) * time.Millisecond
	} else {
		duration = 800 * time.Millisecond
	}

	if err := writeGpio(device, duration); err != nil {
		rsp.ErrRsp(c, -3, fmt.Sprintf("operation failed: %s", err))
		return
	}

	log.Debugf("gpio %s set successfully", device)
	rsp.OkRsp(c)
}

func (s *Service) GetGpio(c *gin.Context) {
	var rsp proto.Response

	conf := config.GetInstance().Hardware

	pwr, err := readGpio(conf.GPIOPowerLED)
	if err != nil {
		rsp.ErrRsp(c, -2, fmt.Sprintf("failed to read power led: %s", err))
		return
	}

	hdd := false
	if conf.Version == config.HWVersionAlpha {
		hdd, err = readGpio(conf.GPIOHDDLed)
		if err != nil {
			rsp.ErrRsp(c, -2, fmt.Sprintf("failed to read hdd led: %s", err))
			return
		}
	}

	data := &proto.GetGpioRsp{
		PWR: pwr,
		HDD: hdd,
	}
	rsp.OkRspWithData(c, data)
}

func writeGpio(device string, duration time.Duration) error {
	if err := os.WriteFile(device, []byte("1"), 0o666); err != nil {
		log.Errorf("write gpio %s failed: %s", device, err)
		return err
	}

	time.Sleep(duration)

	if err := os.WriteFile(device, []byte("0"), 0o666); err != nil {
		log.Errorf("write gpio %s failed: %s", device, err)
		return err
	}

	return nil
}

func readGpio(device string) (bool, error) {
	content, err := os.ReadFile(device)
	if err != nil {
		log.Errorf("read gpio %s failed: %s", device, err)
		return false, err
	}

	contentStr := string(content)
	if len(contentStr) > 1 {
		contentStr = contentStr[:len(contentStr)-1]
	}

	value, err := strconv.Atoi(contentStr)
	if err != nil {
		log.Errorf("invalid gpio content: %s", content)
		return false, nil
	}

	return value == 0, nil
}
