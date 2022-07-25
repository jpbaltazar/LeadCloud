package handlers

import (
	"fmt"
	"strings"
)

func stripUrlPrefix(str, prefix string) (string, error){
	idString := "" + str
	idString = strings.TrimPrefix(idString, prefix)

	if idString == str{
		return "", fmt.Errorf("prefix does not exist in string")
	}

	tokenStartIndex := strings.Index(idString, "?")
	if tokenStartIndex != -1 {
		idString = idString[0:tokenStartIndex]
	}

	return idString, nil
}