package main

var apiSettings struct {
	secret string
	dbConn string
}

type usualStruct struct {
	inner struct {
		int,
		inner2 struct {
		    a string
		}
	}
}

func main() {
	var cars = []struct {
		make  string
		model string
	}{
		{"toyota", "camry"},
	}

	for _, a := range cars {
	    print(a.make)
	}
}
