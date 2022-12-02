package main

type Person struct {
	name        string
	age         int
	city, phone string
}

// A person method
func (p Person) SayHello() {
	print("Hi, I am %s, from %s\n", p.name, p.city)
}

// A person method
func (p *Person) GetDetails() {
	print("[Name: %s, Age: %d]\n", p.name, p.age)
}
