package auth

import (
	"MSPKVM-Server/config"
	"MSPKVM-Server/middleware"
	"MSPKVM-Server/proto"

	"github.com/gin-gonic/gin"
	log "github.com/sirupsen/logrus"
)

func (s *Service) Login(c *gin.Context) {
	var req proto.LoginReq
	var rsp proto.Response

	// authentication disabled
	conf := config.GetInstance()
	if conf.Authentication == "disable" {
		rsp.OkRspWithData(c, &proto.LoginRsp{
			Token: "disabled",
		})
		return
	}

	if err := proto.ParseFormRequest(c, &req); err != nil {
		rsp.ErrRsp(c, -1, "invalid parameters")
		return
	}

	if ok := CompareAccount(req.Username, req.Password); !ok {
		rsp.ErrRsp(c, -2, "invalid username or password")
		return
	}

	token, err := middleware.GenerateJWT(req.Username)
	if err != nil {
		rsp.ErrRsp(c, -3, "generate token failed")
		return
	}

	rsp.OkRspWithData(c, &proto.LoginRsp{
		Token: token,
	})

	log.Debugf("login success, username: %s", req.Username)
}

func (s *Service) Logout(c *gin.Context) {
	conf := config.GetInstance()

	if conf.JWT.RevokeTokensOnLogout {
		config.RegenerateSecretKey()
	}

	var rsp proto.Response
	rsp.OkRsp(c)
}

func (s *Service) GetAccount(c *gin.Context) {
	var rsp proto.Response

	account, err := GetAccount()
	if err != nil {
		rsp.ErrRsp(c, -1, "get account failed")
		return
	}

	rsp.OkRspWithData(c, &proto.GetAccountRsp{
		Username: account.Username,
	})
	log.Debugf("get account successful")
}
