#pragma once

#include <mars/graphics/backend/framebuffer.hpp>
#include <mars/graphics/backend/render_pass.hpp>
#include <mars/graphics/functional/render_pass.hpp>
#include <mars/graphics/object/command_recording.hpp>

namespace mars::graphics::object {

class render_pass_scope {
	const mars::render_pass* m_rp = nullptr;
	const mars::command_buffer* m_cmd = nullptr;

      public:
	render_pass_scope(const mars::render_pass& rp,
			  const command_buffer_recording& rec,
			  const mars::framebuffer& fb,
			  const mars::render_pass_bind_param& params)
	    : m_rp(&rp), m_cmd(&rec.get()) {
		mars::graphics::render_pass_bind(rp, rec, fb, params);
	}

	~render_pass_scope() {
		if (m_rp) mars::graphics::render_pass_unbind(*m_rp, *m_cmd);
	}

	render_pass_scope(const render_pass_scope&) = delete;
	render_pass_scope& operator=(const render_pass_scope&) = delete;
	render_pass_scope(render_pass_scope&&) = delete;
	render_pass_scope& operator=(render_pass_scope&&) = delete;
};

} // namespace mars::graphics::object
