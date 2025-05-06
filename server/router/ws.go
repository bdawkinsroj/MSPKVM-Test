package router

import (
	"github.com/gin-gonic/gin"

	"MSPKVM-Server/middleware"
	"MSPKVM-Server/service/ws"
)

func wsRouter(r *gin.Engine) {
	service := ws.NewService()
	api := r.Group("/api").Use(middleware.CheckToken())

	api.GET("/ws", service.Connect)
}
