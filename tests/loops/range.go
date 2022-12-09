package loops

type Person struct {
	name string
}

func main() {
	strings := []Person{Person{"Mike"}, Person{"Stas"}}
	for i, s := range strings {
		println(i, s)
	}
}
