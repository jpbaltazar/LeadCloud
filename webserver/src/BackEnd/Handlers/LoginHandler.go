package handlers

import (
	"fmt"
	"net/http"
	"time"
)

func LoginPageHandler(w http.ResponseWriter, r *http.Request) {
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

	successful, err := UserAPI.AuthenticateUser(username, password)
	if err != nil || !successful{
		http.Redirect(w, r, "/login.html", 302)
		fmt.Fprintf(w, "Either incorrect credentials or the login is not registered")
		return
	}

	//if successful

	var cookie = http.Cookie{
		Name:       "username",
		Value:      username,
		Expires:    time.Now().Add(365 * 24 * time.Hour),
		HttpOnly:   true,
	}

	http.SetCookie(w, &cookie)

	http.Redirect(w, r, "/devicePage", 302)
	fmt.Fprintf(w, "Welcome %s (%s)\n", username, password)

}