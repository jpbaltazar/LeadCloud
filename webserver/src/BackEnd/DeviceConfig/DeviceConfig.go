package DeviceConfig

import (
	"fmt"
	"math/rand"
	"sync"
	API "webTut/src/APIs"
	"webTut/src/models"
)

var ConfigBindMap = make(map[string]*models.Device) // Config key -> Device representation

type ConfigBinder interface {
	GenConfigKey() string //visible key that shows up in the matrix for config purposes
	AddDevice(key string, device models.Device) error
	GetDevice(key string) *models.Device
	ClaimDevice(key string, params models.Device)
	//ConfigDevice()
}

type DeviceConfigBinder struct {
	ConfigBindMap map[string]*models.Device
	API API.DeviceAPI
	Lock sync.Mutex
}

func (DeviceConfigBinder)GenConfigKey() string{
	const alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"
	const keyLen = 6//8

	var key = ""
	for i := 0; i < keyLen; i++{
		var letter = alphabet[rand.Intn(len(alphabet) - 1)]

		key += "" + string(letter)
	}

	return key
}

func (d DeviceConfigBinder)AddDevice(key string, model *models.Device) error{
	d.Lock.Lock()
	defer d.Lock.Unlock()

	if d.ConfigBindMap[key] != nil{
		return fmt.Errorf("configBindMap key was already taken")
	} else {
		fmt.Printf("Added device with key %s\n", key)
	}
	d.ConfigBindMap[key] = model


	return nil
}

func (d DeviceConfigBinder)GetDevice(key string) *models.Device{
	return d.ConfigBindMap[key]
}

func(d DeviceConfigBinder)ClaimDevice(key string, params models.Device) models.Device{
	d.Lock.Lock()
	defer d.Lock.Unlock()

	//replace everything but the IP
	*d.ConfigBindMap[key] = models.Device{
		Name:	  params.Name,
		Id:       params.Id,
		Owner:    params.Owner,
		IpAdd:    d.ConfigBindMap[key].IpAdd, //conserve the previous one
		Configs:  params.Configs,
		Location: params.Location,
		Status:   params.Status,
	}

	//add this device to the API storage
	d.API.AddDevice(*d.ConfigBindMap[key])

	fmt.Printf("User \"%s\" claimed the device with key %s\n", params.Owner, key)

	toReturn := *d.ConfigBindMap[key]

	//remove
	d.ConfigBindMap[key] = nil

	return toReturn
}