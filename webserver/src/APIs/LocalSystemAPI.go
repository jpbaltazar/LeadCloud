package API

//Future work?

type LocalSystemAPI interface {
	StartConnection()
	CloseConnection()

	RequestScreenshot()
	SendSchedule()
	//SendEvent()
	//SendProgram()
	//
}