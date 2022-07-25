package main

import (
	"fmt"
	"github.com/gorilla/mux"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"go.mongodb.org/mongo-driver/mongo/readpref"
	"golang.org/x/net/context"
	"log"
	"net/http"
	"sync"
	"webTut/src/APIs/MongoDB_APIs"
	"webTut/src/BackEnd/DeviceConfig"
	"webTut/src/BackEnd/Handlers"
	"webTut/src/BackEnd/TCPServer"
	"webTut/src/models"
)

func main() {
	const port = 8080
	fmt.Printf("Starting server on port %d...\n", port)


	//DB setup
	client, err := mongo.Connect(context.TODO(), options.Client().ApplyURI("mongodb://localhost:27017"))
	if err != nil {
		panic(err)
	}

	if err := client.Ping(context.TODO(), readpref.Primary()); err != nil{
		panic(err)
	}



	handlers.ConfigBinder = DeviceConfig.DeviceConfigBinder{
		ConfigBindMap: DeviceConfig.ConfigBindMap,
		API: MongoDB_APIs.MongoDeviceAPI{
			DeviceCollection: client.Database("LeadCloud").Collection("devices"),
		},
		Lock: sync.Mutex{},
	}

	handlers.DeviceAPI = handlers.ConfigBinder.API

	handlers.UserAPI = MongoDB_APIs.MongoUserAPI{
		UserCollection: client.Database("LeadCloud").Collection("users"),
	}

	handlers.ScheduleAPI = MongoDB_APIs.MongoScheduleAPI{
		ScheduleCollection: client.Database("LeadCloud").Collection("schedules"),
		Lock:               sync.Mutex{},
	}
	
	r := mux.NewRouter()

	//r.Handle("/", fileServer)
	//r.Handle("/static/{file}", http.StripPrefix("/static/", fileServer))
	//AUTHENTICATION
	r.HandleFunc("/loginPage", handlers.LoginPageHandler)
	r.HandleFunc("/registerPage", handlers.RegisterPageHandler)

	//DEVICES
	r.HandleFunc("/devicePage", handlers.DevicePageHandler)
	r.HandleFunc("/addDevice", handlers.AddDeviceHandler)
	r.PathPrefix("/removeDevice").HandlerFunc(handlers.RemoveDevice)

	//SCHEDULES
	r.HandleFunc("/schedulePage", handlers.SchedulePageHandler)

	r.HandleFunc("/addSchedule", handlers.AddScheduleHandler)

	r.PathPrefix("/editSchedule").HandlerFunc(handlers.EditScheduleHandler)
	r.HandleFunc("/modifySchedule", handlers.ModifyScheduleHandler)


	r.PathPrefix("/assignScheduleList").HandlerFunc(handlers.AssignScheduleList)
	r.PathPrefix("/assignSchedule").HandlerFunc(handlers.AssignSchedule)

	r.PathPrefix("/removeSchedule").HandlerFunc(handlers.RemoveSchedule)

	//microservice
	r.HandleFunc("/DeviceSelfRegister", handlers.DeviceSelfRegisterHandler).Methods("GET")

	//FILE HOSTING
	fileServer := http.FileServer(http.Dir("./src/frontend"))
	r.PathPrefix("/").Handler(fileServer)


	print("Server started!\n")

	if err := http.ListenAndServe(fmt.Sprintf(":%d", port), r); err != nil {
		log.Fatal(err)
	}
}

/*
func main(){
	e := echo.New()

	e.GET("/", handler)

	g := e.Group("/v1")
	g.POST("/login", Login)
	g.POST("/logout", Logout)

	e.Logger.Fatal(e.Start(":8000"))
}

func handler(c echo.Context) error{
	return c.String(http.StatusOK, "Hello world")
}
*/

func main2(){
	local := TCPServer.LocalSystemConnection{
	}

	//	if err := local.StartConnection("10.42.0.66", "38888"); err != nil{
	if err := local.StartConnection("10.42.0.66"); err != nil{
		print(err)
		return
	}

	var schedule = models.Schedule{
		Name:         "",
		ID:           0,
		Owner:        "",
		JsonSchedule: models.JSONSchedule{},
	}

	if err := local.NewSchedule(schedule); err != nil{
		print(err.Error())
		return
	}

	status, err := local.RequestStatus()
	if err != nil{
		print(err.Error())
		return
	}

	print(status)

	err = local.CloseConnection()
	if err != nil {
		return 
	}

}