package TCPServer

import (
	"encoding/json"
	"fmt"
	"net"
)

func handleConnection(connection net.Conn)  {
	for true {
		var request localSystemRequest

		dec := json.NewDecoder(connection)
		err := dec.Decode(&request)
		if err != nil {
			return
		}

		if request.RequestType == statusUpdate{
			connection.Write([]byte("OK"))
		} else if request.RequestType == sendScheduleMetadata {
			connection.Write([]byte("Not implemented!"))
		} else if request.RequestType == newSchedule {
			connection.Write([]byte("OK"))
		} else if request.RequestType == shutdown {
			connection.Write([]byte("OK"))
		}


	}
}

func startServer(port string){
	fmt.Println("Starting TCP server...")
	listener, err := net.Listen("tcp", ":" + port)
	if err != nil{
		fmt.Println(err)
	}


	for true {
		conn, err := listener.Accept()
		if err != nil{
			print(err)
			break
		}

		go handleConnection(conn)
	}

	/*
	for true {
		var request localSystemRequest

		dec := json.NewDecoder(connection)
		err = dec.Decode(&request)
		if err != nil {
			print(err)
			break
		}

		//starts program
		cmd := exec.Command("pwd") //("JobManager", "pathToSchedule")


		if request.RequestType == statusUpdate {
			_, err = connection.Write([]byte("online"))
			if err != nil{ //timeout and others
				print(err)
				break
			}
		} else if request.RequestType == sendScheduleMetadata{
			panic("not implemented!")
		} else if request.RequestType == newSchedule{
			file, err := os.Open("")
			if err != nil{
				print(err)
				break
			}

			file.Write([]byte(request.JsonSchedule))

			err = cmd.Process.Signal(syscall.SIGUSR1) //refresh schedule
			if err != nil {
				print(err)
				break
			}

			_, _ = connection.Write([]byte("OK"))
		} else if request.RequestType == shutdown{
			//kill jobManager
			err = cmd.Process.Signal(syscall.SIGTERM) //terminate program
			if err != nil {
				print(err)
				break
			}
			//	delete files
			//delete schedule file
			//delete config info (resolution, etc...)

			return
		}

	}*/


}

