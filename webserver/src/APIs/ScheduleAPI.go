package API

import "webTut/src/models"

type ScheduleAPI interface {
	AddSchedule(schedule models.Schedule) error

	FindSchedule(schedule models.Schedule) (*models.Schedule, error)
	FindSchedulesByOwnerAndId(owner string, id int) (*models.Schedule, error)

	FindSchedulesByOwner(owner string) ([]models.Schedule, error)

	RemoveSchedule(toRemove models.Schedule) error
	RemoveSchedules(toRemove []models.Schedule) error
	RemoveAllSchedules(owner string) error
}
