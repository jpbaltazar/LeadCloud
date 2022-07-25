package handlers

import (
	"fmt"
	"net/http"
	"time"
	API "webTut/src/APIs"
)

var UserAPI API.UserAPI

func RegisterPageHandler(w http.ResponseWriter, r *http.Request) {
	print("Trying to register")

	if err := r.ParseForm(); err != nil{
		fmt.Fprintf(w, "ParseForm() err: %v", err)
		return
	}

	username := r.FormValue("username")
	password := r.FormValue("password")

	if len(username) == 0 || len(password) == 0 {
		http.Redirect(w, r, "/login.html", 302)
		fmt.Fprintf(w, "Wrong details\n")
		return
	}

	if err := UserAPI.AddUser(username, password); err != nil{
		http.Redirect(w, r, "/login.html", 302)
		print(err)
		fmt.Printf("Username taken\n")
		//fmt.Fprintf(w, "Username taken\n")
		return
	}

	var usernameCookie = http.Cookie{
		Name:       "username",
		Value:      username,
		Expires:    time.Now().Add(365 * 24 * time.Hour),
		HttpOnly:   true,
	}

	print("Redirecting")

	http.SetCookie(w, &usernameCookie)

	http.Redirect(w, r, "/devicePage", 302)
}