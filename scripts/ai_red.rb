def lookup_offset(local, x, y)
  if x < -2 then
    return :empty
  end
  if x > 2 then
    return :empty
  end
  if y < -2 then
    return :empty
  end
  if y > 2 then
    return :empty
  end

  return local[x + 2][y + 2]
end

def decide_action_red(local, energy)
  if energy > 50 then
    return :east
  end
  if lookup_offset(local, 1, 0) == :food then
    return :east
  end
  if lookup_offset(local, 0, 1) == :food then
    return :north
  end
  if lookup_offset(local, 0, -1) == :food then
    return :south
  end
  
  return :east
end
