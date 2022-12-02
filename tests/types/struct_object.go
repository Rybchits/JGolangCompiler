package main

type candidate struct {
	name       string
	interests  []string
	language   string
	experience bool
}

func main() {
	candidates := []candidate{
		{
			name:       "ravi",
			interests:  []string{"art", "coding", "music", "travel"},
			language:   "golang",
			experience: false,
		},
	}
	println(candidates)
}
