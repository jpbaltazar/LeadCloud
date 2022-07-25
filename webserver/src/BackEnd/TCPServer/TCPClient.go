package TCPServer

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io"
	"net"
	"time"
	"webTut/src/models"
)

//you connect to the server

const PORT_str = "38888"
const PORT_int = 38888

type LocalSystemConnection struct {
	socket 	net.Conn
}

const (
	statusUpdate int = iota //{online, offline, busy} //No meaning for now
	sendScheduleMetadata  		 	//time of schedule
	newSchedule					 	//receive a schedule to replace the current one
	shutdown					 	//delete all config files and restart
	newConfig
	closeConnection
	screenshot
)

//TODO extract functions to remove duplicated code

type localSystemRequest struct {
	RequestType int
	JsonSchedule string
	Config string
}

//untested!
func sendJson(request localSystemRequest, writer io.Writer) error{
	enc := json.NewEncoder(writer)
	if err := enc.Encode(request); err != nil{
		return err
	}
	return nil
}



func (l *LocalSystemConnection)StartConnection(ip string) error{
	fmt.Println("Connecting to " + ip + ":" + PORT_str)

	connection, err := net.Dial("tcp", ip + ":" + PORT_str)
	if err != nil {
		return err
	}

	l.socket = connection
	return nil
}

func (l *LocalSystemConnection)NewConfig(config string) error{
	var request = localSystemRequest{
		RequestType: newConfig,
		Config: config,
	}

	enc := json.NewEncoder(l.socket)
	if err := enc.Encode(request); err != nil{
		return err
	}

	err := l.socket.SetReadDeadline(time.Now().Add(time.Second))
	if err != nil {
		return err
	}

	var buf = make([]byte, 4096)

	n, err := l.socket.Read(buf)
	if err != nil{
		return err
	}

	if string(buf[:n]) != "OK" {
		return fmt.Errorf("response was not 'OK'")
	}

	return nil
}

func (l LocalSystemConnection)RequestStatus() (string, error){
	var request = localSystemRequest{
		RequestType: statusUpdate,
	}

	enc := json.NewEncoder(l.socket)
	if err := enc.Encode(request); err != nil{
		return "", err
	}

	err := l.socket.SetReadDeadline(time.Now().Add(time.Second))
	if err != nil {
		return "", err
	}

	var buf = make([]byte, 4096)

	n, err := l.socket.Read(buf)
	if err != nil{
		return "", err
	}

	responseString := string(buf[:n])
	return responseString, nil
}

func (l LocalSystemConnection)SendScheduleMetadata() (string, error) {
	var request = localSystemRequest{
		RequestType: sendScheduleMetadata,
	}

	enc := json.NewEncoder(l.socket)
	if err := enc.Encode(request); err != nil{
		return "", err
	}

	err := l.socket.SetReadDeadline(time.Now().Add(time.Second))
	if err != nil {
		return "", err
	}

	var buf = make([]byte, 4096)

	n, err := l.socket.Read(buf)
	if err != nil{
		return "", err
	}


	return string(buf[:n]), nil
}

func (l LocalSystemConnection)NewSchedule(schedule models.Schedule) error {
	jsonSchedule, err := json.Marshal(schedule.JsonSchedule)
	if err != nil{
		return err
	}
	scheduleString := string(jsonSchedule)

	var request = localSystemRequest{
		RequestType: newSchedule,
		JsonSchedule: scheduleString,
	}

	requestBytes, err := json.Marshal(request)
	if err != nil{
		return err
	}

	_, err = l.socket.Write(requestBytes)
	if err != nil {
		return err
	}

	err = l.socket.SetReadDeadline(time.Now().Add(time.Second))
	if err != nil {
		return err
	}

	var buf = make([]byte, 4096)

	n, err := l.socket.Read(buf)
	if err != nil{
		return err
	}

	if string(buf[:n]) != "OK" {
		return fmt.Errorf("response was not 'OK'")
	}

	return nil
}

func (l LocalSystemConnection)Shutdown() error {
	var request = localSystemRequest{
		RequestType: shutdown,
	}

	requestBytes, err := json.Marshal(request)
	if err != nil{
		return err
	}

	_, err = l.socket.Write(requestBytes)
	if err != nil{
		return err
	}

	err = l.socket.SetReadDeadline(time.Now().Add(time.Second))
	if err != nil {
		return err
	}

	var buf = make([]byte, 4096)

	n, err := l.socket.Read(buf)
	if err != nil{
		return err
	}


	if string(buf[:n]) != "OK" {
		return fmt.Errorf("response was not 'OK'")
	}

	return nil
}

func (l *LocalSystemConnection)CloseConnection() error {
	var request = localSystemRequest{
		RequestType: closeConnection,
	}

	requestBytes, err := json.Marshal(request)
	if err != nil{
		return err
	}

	_, err = l.socket.Write(requestBytes)
	if err != nil {
		return err
	}

	err = l.socket.Close()
	if err != nil {
		return err
	}

	l.socket = nil
	return nil
}

func (l LocalSystemConnection)Screenshot() ([]uint32, error){
	var request = localSystemRequest{
		RequestType: screenshot,
	}

	requestBytes, err := json.Marshal(request)
	if err != nil{
		return nil, err
	}

	_, err = l.socket.Write(requestBytes)
	if err != nil{
		return nil, err
	}

	buf32 := make([]uint32, 32*64)
	err = binary.Read(l.socket, binary.LittleEndian, &buf32)
	if err != nil{
		return nil, err
	}

	return buf32, nil
}