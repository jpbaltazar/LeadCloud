package MongoDB_APIs

import (
	"context"
	"fmt"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/mongo"
	"sync"
	"webTut/src/models"
)

type MongoScheduleAPI struct {
	ScheduleCollection *mongo.Collection
	Lock               sync.Mutex
}

func (m MongoScheduleAPI) isIDTaken(owner string, id int) bool {

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

	var foundUser models.Schedule
	err := m.ScheduleCollection.FindOne(context.TODO(), filter).Decode(&foundUser)
	if err != nil { //not found
		return false
	}
	return true
}

func (m MongoScheduleAPI) AddSchedule(schedule models.Schedule) error {
	//TODO since schedules have IDs, a lock should be added to prevent different schedules from having the same ids
	m.Lock.Lock()
	defer m.Lock.Unlock()

	if m.isIDTaken(schedule.Owner, schedule.ID) {
		return fmt.Errorf("id already taken")
	}

	_, err := m.ScheduleCollection.InsertOne(context.TODO(), schedule)
	if err != nil {
		return err
	}

	return nil
}

func (m MongoScheduleAPI) FindSchedule(schedule models.Schedule) (*models.Schedule, error) {
	var result models.Schedule
	if err := m.ScheduleCollection.FindOne(context.TODO(), schedule).Decode(&result); err != nil {
		return nil, err
	}

	var foundUser models.Schedule
	err := m.ScheduleCollection.FindOne(context.TODO(), schedule).Decode(&foundUser)
	if err != nil {
		return nil, err
	}

	return &foundUser, nil
}

func (m MongoScheduleAPI) FindSchedulesByOwner(owner string) ([]models.Schedule, error) {
	if owner == "" {
		return nil, fmt.Errorf("empty owner name")
	}

	filter := bson.D{
		{"$and",
			bson.A{
				bson.D{
					{"owner", owner},
				},
			},
		},
	}

	cursor, err := m.ScheduleCollection.Find(context.TODO(), filter)
	if err != nil {
		return nil, err
	}

	var results []models.Schedule
	if err = cursor.All(context.TODO(), &results); err != nil {
		return nil, err
	}

	return results, nil
}

func (m MongoScheduleAPI) FindSchedulesByOwnerAndId(owner string, id int) (*models.Schedule, error) {
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

	var foundUser models.Schedule
	err := m.ScheduleCollection.FindOne(context.TODO(), filter).Decode(&foundUser)
	if err != nil { //not found or others
		return nil, err
	}
	return &foundUser, nil
}

func (m MongoScheduleAPI) RemoveSchedule(toRemove models.Schedule) error {
	m.Lock.Lock()
	defer m.Lock.Unlock()
	_, err := m.ScheduleCollection.DeleteOne(context.TODO(), toRemove)
	if err != nil {
		return err
	}

	return nil
}

func (m MongoScheduleAPI) RemoveSchedules(toRemove []models.Schedule) error {
	m.Lock.Lock()
	defer m.Lock.Unlock()
	_, err := m.ScheduleCollection.DeleteMany(context.TODO(), toRemove)
	if err != nil {
		return err
	}

	return nil
}

func (m MongoScheduleAPI) RemoveAllSchedules(owner string) error {
	m.Lock.Lock()
	defer m.Lock.Unlock()

	filter := bson.D{
		{"$and",
			bson.A{
				bson.D{
					{"Owner", owner},
				},
			},
		},
	}

	_, err := m.ScheduleCollection.DeleteMany(context.TODO(), filter)
	if err != nil {
		return err
	}

	return nil
}
