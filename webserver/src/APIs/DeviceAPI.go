package API

import (
	"webTut/src/models"
)


type DeviceAPI interface{
	AddDevice(models.Device) error

	FindDevice(device models.Device) (*models.Device, error)
	FindDeviceByOwnerAndId(owner string, id int) (*models.Device, error)
	FindDevicesByOwner(owner string) ([]models.Device, error)
	//FindAllDevices() (*[]models.Device, error)

	RemoveDevice(toRemove models.Device) error
	RemoveDevices(toRemove []models.Device) error
	RemoveAllDevices(owner string) error
}


