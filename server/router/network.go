package router

import (
	"github.com/gin-gonic/gin"

	"MSPKVM-Server/middleware"
	"MSPKVM-Server/service/network"
)

func networkRouter(r *gin.Engine) {
	service := network.NewService()

	r.POST("/api/network/wifi", service.ConnectWifi) // connect Wi-Fi

	api := r.Group("/api").Use(middleware.CheckToken())

	api.POST("/network/wol", service.WakeOnLAN)           // wake on lan
	api.GET("/network/wol/mac", service.GetMac)           // get mac list
	api.DELETE("/network/wol/mac", service.DeleteMac)     // delete mac
	api.POST("/network/wol/mac/name", service.SetMacName) // set mac name
	api.GET("/network/wifi", service.GetWifi)             // get Wi-Fi information
}
