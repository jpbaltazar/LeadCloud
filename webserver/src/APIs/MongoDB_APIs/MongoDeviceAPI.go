package MongoDB_APIs

import (
	"context"
	"fmt"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/mongo"
	"webTut/src/models"
)

type MongoDeviceAPI struct {
	DeviceCollection *mongo.Collection
}



func (API MongoDeviceAPI) AddDevice(d models.Device) error{
	/*device := bson.D{
		{"Id", d.Id},
		{"Name", d.Name},
		{"Owner", d.Owner},
		{"IpAdd", d.IpAdd},
		{"Configs", d.Configs},
		{"Location", d.Location},
		{"Status", "online"},
	}*/

	_, err := API.DeviceCollection.InsertOne(context.TODO(), d)
	if err != nil {
		return err
	}
	return nil
}

func (d MongoDeviceAPI) FindDevice(device models.Device) (*models.Device, error){

	var result models.Device
	if err := d.DeviceCollection.FindOne(context.TODO(), device).Decode(&result); err != nil{
		return nil, err
	}

	return &result, nil
}

func (API MongoDeviceAPI) FindDeviceByOwnerAndId(owner string, id int) (*models.Device, error) {
	if owner == "" {
		return nil, fmt.Errorf("empty owner name")
	}

	filter := bson.D{
		{"$and",
			bson.A{
				bson.D{
					{"owner", owner},
				},
				bson.D{
					{"id", id},
				},
			},
		},
	}

	var foundDevice models.Device
	err := API.DeviceCollection.FindOne(context.TODO(), filter).Decode(&foundDevice)
	if err != nil{ //not found
		return nil, err
	}

	return &foundDevice, nil
}

func (d MongoDeviceAPI) FindDevicesByOwner(Owner string) ([]models.Device, error) {
	if Owner == "" {
		return nil, fmt.Errorf("empty owner name")
	}

	filter := bson.D{
		{"$and",
			bson.A{
				bson.D{
					{"owner", Owner},
				},
			},
		},
	}

	cursor, err := d.DeviceCollection.Find(context.TODO(), filter)
	if err != nil{
		return nil, err
	}

	var results []models.Device
	if err = cursor.All(context.TODO(), &results); err != nil{
		return nil, err
	}

	return results, nil
}


func (d MongoDeviceAPI) RemoveDevice(device models.Device) error{
	_, err := d.DeviceCollection.DeleteOne(context.TODO(), device)
	if err != nil{
		return err
	}

	return nil
}

func (d MongoDeviceAPI) RemoveDevices(devices []models.Device) error{
	_, err := d.DeviceCollection.DeleteMany(context.TODO(), devices)
	if err != nil{
		return err
	}

	return nil
}

func (d MongoDeviceAPI) RemoveAllDevices(Owner string) error{
	filter := bson.D{
		{"$and",
			bson.A{
				bson.D{
					{"Owner", Owner},
				},
			},
		},
	}


	_, err := d.DeviceCollection.DeleteMany(context.TODO(), filter)
	if err != nil{
		return err
	}

	return nil
}