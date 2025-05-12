def decide_action(local, energy)
  # Basic behavior: move randomly
  [:stay, :north, :south, :east, :west].sample
end