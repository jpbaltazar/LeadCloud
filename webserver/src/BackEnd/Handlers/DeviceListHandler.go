package handlers

import (
	"bytes"
	"net/http"
	"strings"
	"text/template"
	"webTut/src/APIs"
	"webTut/src/BackEnd/TCPServer"
)

//visual model only

type MatrixVisualization struct {
	MHeight string
	MWidth string
	Scale string
	Matrix string
}

type DeviceTableEntry struct {
	Name string
	Status string //could be an enum
	Configuration string
	Location string
	ID int
	Matrix   MatrixVisualization
}

var DeviceAPI API.DeviceAPI

func DevicePageHandler(w http.ResponseWriter, r *http.Request){
	//get the actual entries instead of generating them

	usernameCookie, err := r.Cookie("username")
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	var username = usernameCookie.Value

	devices, err := DeviceAPI.FindDevicesByOwner(username)
	if err != nil {
		http.Redirect(w, r, "/login.html", 302)
		return
	}

	entries := ""

	for d := range devices {
		var device = devices[d]

		status := ""

		var deviceConn = TCPServer.LocalSystemConnection{}
		err := deviceConn.StartConnection(device.IpAdd)
		if err != nil {
			status = "offline"
		} else {
			status, err = deviceConn.RequestStatus()
			if err != nil{
				print(err.Error() + "\n")
				status = "unknown"
			}

			err := deviceConn.CloseConnection()
			if err != nil {
				return 
			}
		}

		configFields := strings.Split(device.Configs, "x")

		dte := DeviceTableEntry{
			Name:          device.Name,
			Status:        status,
			Configuration: device.Configs,
			Location:      device.Location,
			ID:			   device.Id,
			Matrix:        MatrixVisualization{
				MHeight: configFields[0],
				MWidth: configFields[1],
				Scale: "12",
			},
		}

		/* //Too heavy!!!

		err = deviceConn.StartConnection(device.IpAdd)
		if err != nil {
			return
		}
		
		matrix, err := deviceConn.Screenshot()
		if err != nil {
			return
		}

		err = deviceConn.CloseConnection()
		if err != nil {
			return 
		}

		for i := 0; i < 32*64; i++{
			dte.Matrix.Matrix += fmt.Sprintf("0x%08x ", matrix[i])
		}*/

		//Should obtain it from the device rather than doing it like this
		//red debug cross
		for i := 0; i < 32; i++{
			for j := 0; j < 64; j++{
				if (i * 2 == j - 1) || (i * 2 == j) || (i * 2 == 63 - j) || (i * 2 == 62 - j){
					dte.Matrix.Matrix += "#FF0000 "
				} else{
					dte.Matrix.Matrix += "#000000 "
				}
			}
		}

		buf := bytes.Buffer{}

		entry, err := template.ParseFiles("./src/frontend/templates/deviceEntry.html")
		if err != nil{
			print("Error! %d\n", err)
		}

		_ = entry.Execute(&buf, dte)
		if err != nil{
			print("Error in Execute %d\n, err")
		}
		entries += buf.String() //added a row of info
	}


	deviceTemplate, _ := template.ParseFiles("./src/frontend/templates/devicesT.html")
	_ = deviceTemplate.Execute(w, entries)
}
