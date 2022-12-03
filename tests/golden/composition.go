package main

type report1 struct {
	sol         int
	temperature temperature // Поле temperature является структурой типа temperature
	location    location
}

type report2 struct {
	sol         int
	temperature // Тип temperature встроен в report
	location
}

type report3 struct {
	sol // Новый alias для int - sol
	location
	temperature
}

type sol int

type temperature struct{ high, low celsius }

type location struct {
	lat, long float64
}

type celsius float64

func main() {
	/*bradbury := location{-4.5895, 137.4417}
	t := temperature{high: -1.0, low: -78.0}
	report1_ex := report1{sol: 15, temperature: t, location: bradbury}
	report2_ex := report2{sol: 20, temperature: t, location: bradbury}
	report3_ex := report3{sol: 25, temperature: t, location: bradbury}

	print(report1_ex.temperature.high)
	print(report2_ex.location.lat)
	print(report3_ex.lat)*/
}
