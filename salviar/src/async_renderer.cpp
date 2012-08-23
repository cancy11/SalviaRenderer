#include <salviar/include/async_renderer.h>

#include <salviar/include/renderer.h>
#include <eflib/include/memory/bounded_buffer.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/utility/addressof.hpp>
#include <boost/thread/mutex.hpp>
#include <eflib/include/platform/boost_end.h>

using std::vector;

BEGIN_NS_SALVIAR();

EFLIB_DECLARE_CLASS_SHARED_PTR(renderer);

class async_renderer: public renderer
{
public:
	async_renderer(): cmds_(32) {}

	//inherited
	virtual result set_input_layout(h_input_layout const& layout)
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_input_layout, impl_, layout);
		cmds_.push_front(cmd);
		return result::ok;
	}

	virtual result set_vertex_buffers(
		size_t starts_slot,
		size_t buffers_count, h_buffer const* buffers,
		size_t const* strides, size_t const* offsets
		)
	{
		vector<h_buffer>	buffers_vector(buffers, buffers+buffers_count);
		vector<size_t>		strides_vector(strides, strides+buffers_count);
		vector<size_t>		offsets_vector(offsets, offsets+buffers_count);

		boost::function<result()> cmd = boost::bind(
			&async_renderer::set_vertex_buffer_impl,
			this, starts_slot, buffers_vector, strides_vector, offsets_vector);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result set_index_buffer(h_buffer const& hbuf, format index_fmt)
	{
		boost::function<result()> cmd = boost::bind(
			&renderer::set_index_buffer, impl_, hbuf, index_fmt );
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual h_buffer get_index_buffer() const;
	virtual format get_index_format() const;

	virtual result set_primitive_topology(primitive_topology primtopo)
	{
		boost::function<result()> cmd = boost::bind(
			&renderer::set_primitive_topology, impl_, primtopo );
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual primitive_topology get_primitive_topology() const;

	virtual result set_vertex_shader(h_vertex_shader const& hvs)
	{
		boost::function<result()> cmd = boost::bind(
			&renderer::set_vertex_shader, impl_, hvs );
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual h_vertex_shader get_vertex_shader() const;
	
	virtual result set_vertex_shader_code( boost::shared_ptr<shader_code> const& vsc )
	{
		boost::function<result()> cmd = boost::bind(
			&renderer::set_vertex_shader_code, impl_, vsc );
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual boost::shared_ptr<shader_code> get_vertex_shader_code() const;

	virtual result set_vs_variable_value( std::string const& name, void const* pvariable, size_t sz )
	{
		char const* pbytes = static_cast<char const*>(pvariable);
		vector<char> data_vector(pbytes, pbytes+sz);
		boost::function<result()> cmd =
			boost::bind(&async_renderer::set_vs_variable_value_impl, this, name, data_vector);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result set_vs_variable_pointer( std::string const& name, void const* pvariable, size_t sz )
	{
		char const* pbytes = static_cast<char const*>(pvariable);
		vector<char> data_vector(pbytes, pbytes+sz);
		boost::function<result()> cmd =
			boost::bind(&async_renderer::set_vs_variable_pointer_impl, this, name, data_vector);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result set_vs_sampler( std::string const& name, h_sampler const& samp )
	{
		boost::function<result()> cmd =
			boost::bind(&renderer::set_vs_sampler, impl_, name, samp);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result set_rasterizer_state(h_rasterizer_state const& rs)
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_rasterizer_state, impl_, rs);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual h_rasterizer_state get_rasterizer_state() const;

	virtual result set_depth_stencil_state(h_depth_stencil_state const& dss, int32_t stencil_ref)
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_depth_stencil_state, impl_, dss, stencil_ref);
		cmds_.push_front(cmd);

		return result::ok;
	}
	virtual const h_depth_stencil_state& get_depth_stencil_state() const;
	virtual int32_t get_stencil_ref() const;

	virtual result set_pixel_shader(h_pixel_shader const& hps)
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_pixel_shader, impl_, hps);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual h_pixel_shader get_pixel_shader() const;

	virtual result set_pixel_shader_code( boost::shared_ptr<shader_code> const& psc )
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_pixel_shader_code, impl_, psc);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual boost::shared_ptr<shader_code> get_pixel_shader_code() const;

	virtual result set_ps_variable( std::string const& name, void const* data, size_t sz )
	{
		char const* pbytes = static_cast<char const*>(data);
		vector<char> data_vector(pbytes, pbytes+sz);
		boost::function<result()> cmd = boost::bind(&async_renderer::set_ps_variable_impl, this, name, data_vector);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result set_ps_sampler( std::string const& name, h_sampler const& samp )
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_ps_sampler, impl_, name, samp);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result set_blend_shader(h_blend_shader const& hbs)
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_blend_shader, impl_, hbs);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual h_blend_shader get_blend_shader();

	virtual result set_viewport(const viewport& vp)
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_viewport, impl_, vp);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual viewport get_viewport() const;

	virtual result set_framebuffer_size(size_t width, size_t height, size_t num_samples)
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_framebuffer_size, impl_, width, height, num_samples);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual eflib::rect<size_t> get_framebuffer_size() const;

	virtual result set_framebuffer_format(pixel_format pxfmt)
	{
		boost::function<result()> cmd = boost::bind(&renderer::set_framebuffer_format, impl_, pxfmt);
		cmds_.push_front(cmd);

		return result::ok;
	};

	virtual pixel_format get_framebuffer_format(pixel_format pxfmt) const;

	virtual result set_render_target_available(render_target tar, size_t target_index, bool valid)
	{
		boost::function<result()> cmd =
			boost::bind(&renderer::set_render_target_available, impl_, tar, target_index, valid);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual bool get_render_target_available(render_target tar, size_t target_index) const;

	virtual h_framebuffer get_framebuffer() const;

	virtual result set_render_target(render_target tar, size_t target_index, h_surface const& surf)
	{
		boost::function<result()> cmd =
			boost::bind(&renderer::set_render_target, impl_, tar, target_index, surf);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual h_input_layout create_input_layout(
		input_element_desc const* elem_descs, size_t elems_count,
		h_shader_code const& vs )
	{
		return impl_->create_input_layout(elem_descs, elems_count, vs);
	}
	
	virtual h_input_layout create_input_layout(
		input_element_desc const* elem_descs, size_t elems_count,
		h_vertex_shader const& vs )
	{
		return impl_->create_input_layout(elem_descs, elems_count, vs);
	}

	virtual h_buffer	create_buffer(size_t size);
	virtual h_texture	create_tex2d(size_t width, size_t height, size_t num_samples, pixel_format fmt);
	virtual h_texture	create_texcube(size_t width, size_t height, size_t num_samples, pixel_format fmt);
	virtual h_sampler	create_sampler(const sampler_desc& desc);

	virtual result draw(size_t startpos, size_t primcnt)
	{
		boost::function<result()> cmd =
			boost::bind(&renderer::draw, impl_, startpos, primcnt);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result draw_index(size_t startpos, size_t primcnt, int basevert)
	{
		boost::function<result()> cmd =
			boost::bind(&renderer::draw_index, impl_, startpos, primcnt, basevert);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result clear_color(size_t target_index, const color_rgba32f& c)
	{
		boost::function<result()> cmd =
			boost::bind(&renderer::clear_color, impl_, target_index, c);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result clear_depth(float d)
	{
		boost::function<result()> cmd = 
			boost::bind(&renderer::clear_depth, impl_, d);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result clear_stencil(uint32_t s)
	{
		boost::function<result()> cmd = 
			boost::bind(&renderer::clear_stencil, impl_, s);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result clear_color(size_t target_index, const eflib::rect<size_t>& rc, const color_rgba32f& c)
	{
		boost::function<result()> cmd = 
			boost::bind(&renderer::clear_color, impl_, target_index, rc, c);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result clear_depth(const eflib::rect<size_t>& rc, float d)
	{
		boost::function<result()> cmd = 
			boost::bind(&renderer::clear_depth, impl_, rc, d);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result clear_stencil(const eflib::rect<size_t>& rc, uint32_t s)
	{
		boost::function<result()> cmd = 
			boost::bind(&renderer::clear_stencil, impl_, rc, s);
		cmds_.push_front(cmd);

		return result::ok;
	}

	virtual result present()
	{
		boost::function<result()> cmd = boost::bind(&renderer::present, impl_);
		cmds_.push_front(cmd);

		return result::ok;
	}
	
	void run()
	{
		shared_impl_ = create_software_renderer( NULL, h_device() );
		impl_ = shared_impl_.get();

		rendering_thread_ = boost::thread( &async_renderer::working, this );
	}
private:
	virtual result set_vertex_buffer_impl(
		size_t starts_slot,
		vector<h_buffer> const& buffers,
		vector<size_t> const& strides,
		vector<size_t> const& offsets
		)
	{
		impl_->set_vertex_buffers(
			starts_slot, buffers.size(),
			boost::addressof(buffers[0]), boost::addressof(strides[0]), boost::addressof(offsets[0])
		);

		return result::ok;
	}

	virtual result set_vs_variable_value_impl( std::string const& name, vector<char> const& data )
	{
		impl_->set_vs_variable_value( name, static_cast<void const*>( boost::addressof(data[0]) ), data.size() );
		return result::ok;
	}

	virtual result set_vs_variable_pointer_impl( std::string const& name, vector<char> const& data )
	{
		impl_->set_vs_variable_pointer( name, static_cast<void const*>( boost::addressof(data[0]) ), data.size() );
		return result::ok;
	}

	virtual result set_ps_variable_impl( std::string const& name, vector<char> const& data )
	{
		impl_->set_ps_variable( name, boost::addressof(data[0]), data.size() );
		return result::ok;
	}

	result release()
	{
		if ( rendering_thread_.joinable() )
		{
			cmds_.push_front( boost::bind(&async_renderer::exit_rendering_thread, this) );
			rendering_thread_.join();
		}
		return result::ok;
	}

	result exit_rendering_thread()
	{
		// Running in working thread.
		shared_impl_.reset();
		impl_ = NULL;
		return result::ok;
	}

	void working()
	{
		while( shared_impl_ )
		{
			boost::function<result()> cmd;
			cmds_.pop_back(&cmd);
			cmd();
		}
	}

	eflib::bounded_buffer< boost::function<result()> > cmds_;
	
	boost::thread	rendering_thread_;
	renderer_ptr	shared_impl_;
	renderer*		impl_;
};

renderer_ptr create_async_renderer()
{
	boost::shared_ptr<async_renderer> ret( new async_renderer() );
	ret->run();
	return ret;
}

END_NS_SALVIAR();