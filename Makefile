.PHONY: all container netns memcgroup cpucgroup clean run

all: container netns memcgroup cpucgroup

container:
	cd src && make

memcgroup:
	cd test/cgroup_test/mem_cgroup && make

cpucgroup:
	cd test/cgroup_test/cpu_cgroup && make

netns:
	cd test/namespace_test/net && make

clean:
	cd src && make clean
	cd test/cgroup_test/mem_cgroup && make clean
	cd test/cgroup_test/cpu_cgroup && make clean
	cd test/namespace_test/net && make clean

run: container
	cd src && make run