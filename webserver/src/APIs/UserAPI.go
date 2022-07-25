package API

import "webTut/src/models"

type UserAPI interface {
	AddUser(username, password string) error

	FindUserName(username string) (* models.User, error)

	AuthenticateUser(username, password string) (bool, error)

	RemoveUser(username string) error
}

