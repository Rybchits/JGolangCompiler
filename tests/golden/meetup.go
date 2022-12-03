package main

type People interface {
	SayHello()
	GetDetails()
}
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
func (p Person) GetDetails() {
	print("[Name: %s, Age: %d, City: %s, Phone: %s]\n", p.name, p.age, p.city, p.phone)
}

type Speaker struct {
	Person     //type embedding for composition
	speaksOn   []string
	pastEvents []string
}

// overrides GetDetails
func (s Speaker) GetDetails() {
	//Call person GetDetails
	s.Person.GetDetails()
	println("Speaker talks on following technologies:")
	for (_, value := range s.speaksOn) {
		println(value)
	}
	println("Presented on the following conferences:")
	for (_, value := range s.pastEvents) {
		println(value)
	}
}

type Organizer struct {
	Person  //type embedding for composition
	meetups []string
}

// overrides GetDetails
func (o Organizer) GetDetails() {
	//Call person GetDetails
	o.Person.GetDetails()
	println("Organizer, conducting following Meetups:")
	for (_, value := range o.meetups) {
		println(value)
	}
}

type Attendee struct {
	Person    //type embedding for composition
	interests []string
}
type Meetup struct {
	location string
	city     string
	date     string
	people   []People
}

func (m Meetup) MeetupPeople() {
	for (_, v := range m.people) {
		v.SayHello()
		v.GetDetails()
	}
}
func main() {
	shiju := Speaker{Person{"Shiju", 35, "Kochi", "+91-94003372xx"},
		[]string{"Go", "Docker", "Azure", "AWS"},
		[]string{"FOSS", "JSFOO", "MS TechDays"}}
	satish := Organizer{Person{"Satish", 35, "Pune", "+91-94003372xx"},
		[]string{"Gophercon", "RubyConf"}}
	alex := Attendee{Person{"Alex", 22, "Bangalore", "+91-94003672xx"},
		[]string{"Go", "Ruby"}}
	meetup := Meetup{
		"Royal Orchid",
		"Bangalore",
		"31.12.2022",
		[]People{shiju, satish, alex},
	}
	//get details of meetup people
	meetup.MeetupPeople()
}
