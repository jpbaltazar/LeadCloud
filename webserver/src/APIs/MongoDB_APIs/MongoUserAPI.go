package MongoDB_APIs

import (
	"context"
	"fmt"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/mongo"
	"golang.org/x/crypto/bcrypt"
	"webTut/src/models"
)

type MongoUserAPI struct {
	UserCollection *mongo.Collection
}

func (D MongoUserAPI) AddUser(username, password string) error {
	foundUser, err := D.FindUserName(username)
	if err != nil{
		return err
	}

	if foundUser != nil{
		return fmt.Errorf("username taken")
	}

	passBytes, err := bcrypt.GenerateFromPassword([]byte(password), 14)
	if err != nil{
		return err
	}

	user := models.User{
		Username: username,
		Password: passBytes,
	}

	_, err = D.UserCollection.InsertOne(context.TODO(), user)
	if err != nil {
		return err
	}

	return nil
}

func (D MongoUserAPI) FindUserName(username string) (* models.User, error) {
	var foundUser models.User
	err := D.UserCollection.FindOne(context.TODO(), bson.D{{"username", username}}).Decode(&foundUser)
	if err == mongo.ErrNoDocuments{
		return nil, nil
	} else if err != nil{
		return nil, err
	}

	return &foundUser, nil
}

func (D MongoUserAPI) AuthenticateUser(username, password string) (bool, error) {
	user, err := D.FindUserName(username)
	if err != nil{
		return false, err
	}

	if user == nil {
		return false, fmt.Errorf("username not found")
	}

	err = bcrypt.CompareHashAndPassword(user.Password, []byte(password))
	return err == nil, nil
}

func (D MongoUserAPI) RemoveUser(username string) error {

	user, err := D.FindUserName(username)
	if err != nil {
		return err
	}

	if user == nil{
		return nil
	}

	_, err = D.UserCollection.DeleteOne(context.TODO(), user)
	if err != nil{
		return err
	}

	return nil
}



