module StateHelpers
  def states_are_equal(desired_state, retrieved_state)
    expect(retrieved_state).to include(*desired_state.keys)
    expect(retrieved_state.select { |x| desired_state.include?(x) } ).to eq(desired_state)
  end
end